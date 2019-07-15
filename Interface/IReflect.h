// IReflect.h
// By PaintDream (paintdream@paintdream.com)
// 2014-7-16
//

#ifndef __IREFLECT_H__
#define __IREFLECT_H__

#include "../PaintsNow.h"
#include "IType.h"
#include "../Template/TObject.h"
#include "../Template/TProxy.h"
#include "../Template/TFactory.h"
#include "../Template/TAtomic.h"
#include "IThread.h"
#include <vector>
#include <cstring>
#include <map>
#include <cassert>
#include <iterator>
#ifndef _MSC_VER
#include <typeinfo>
#endif

#ifdef __GNUG__
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif


#ifdef _MSC_VER
#pragma warning(disable:4172)
#endif

namespace PaintsNow {
	class IStreamBase;
	class IReflectObject;
	class IUniqueAllocator;

	// Runtime class info
	struct IUniqueInfo {
		virtual ~IUniqueInfo() {}
		virtual size_t GetSize() const;
		virtual IReflectObject* Create() const;
		virtual IUniqueAllocator* GetAllocator() const;
		String GetSubName() const;
		static inline bool CompareInterface(const std::pair<IUniqueInfo*, size_t>& lhs, const std::pair<IUniqueInfo*, size_t>& rhs) {
			return lhs.first < rhs.first;
		}

		bool IsClass(IUniqueInfo* info) const {
			return std::binary_find(interfaces.begin(), interfaces.end(), std::make_pair(info, (size_t)0), &IUniqueInfo::CompareInterface) != interfaces.end();
		}

		enum INT_STATE { IS_INTEGER, NOT_INTEGER, TBD };

		IUniqueAllocator* allocator;
		size_t size;
		String typeName;
		TWrapper<IReflectObject*> creator;
		std::vector<std::pair<IUniqueInfo*, size_t> > interfaces; // Derivations
	};

	// Quick wrapper for runtime class info
	struct Unique {
		Unique(IUniqueInfo* f = nullptr);
		bool operator == (const Unique& unique) const;
		bool operator != (const Unique& unique) const;
		bool operator < (const Unique& unique) const;
		const IUniqueInfo* operator -> () const;
		IUniqueInfo* operator -> ();

		operator IUniqueInfo* () const {
			return info;
		}

		IUniqueInfo* info;
	};

	class IUniqueAllocator {
	public:
		virtual ~IUniqueAllocator();
		virtual IUniqueInfo* Alloc(const String& name, size_t size) = 0;
		virtual IUniqueInfo* Get(const String& name) = 0;
	};

	class UniqueAllocator : public IUniqueAllocator {
	public:
		UniqueAllocator();
		virtual ~UniqueAllocator();
		virtual IUniqueInfo* Alloc(const String& name, size_t size);
		virtual IUniqueInfo* Get(const String& name);

	private:
		TAtomic<int32_t> critical;
		std::map<String, IUniqueInfo> mapType;
	};

	IUniqueAllocator* GetGlobalUniqueAllocator();
	void SetGlobalUniqueAllocator(IUniqueAllocator* alloc);

	template <class T>
	struct UniqueTypeEx {
		static std::string Demangle(const char* name) {
			String className;
		#ifdef __GNUG__
			int status = -4; // some arbitrary value to eliminate the compiler warning
			// enable c++11 by passing the flag -std=c++11 to g++
			std::unique_ptr<char, void(*)(void*)> res {
				abi::__cxa_demangle(name, nullptr, nullptr, &status),
					std::free
			};

			className = (status==0) ? res.get() : name;
		#else
			className = name;
		#endif

			// remove 'class ' or 'struct ' prefix
			const String skipPrefixes[] = { "class ", "struct " };
			for (size_t i = 0; i < sizeof(skipPrefixes) / sizeof(skipPrefixes[0]); i++) {
				RemovePatterns(className, skipPrefixes[i]);
			}

			return className;
		}

		static void RemovePatterns(String& s, const String& p) {
			size_t size = p.size();
			for (size_t i = s.find(p); i != String::npos; i = s.find(p))
				s.erase(i, size);
		}

		static Unique Get(IUniqueAllocator* allocator = GetGlobalUniqueAllocator()) {
			assert(allocator != nullptr);
			static String className = Demangle(typeid(T).name());
			static IUniqueInfo* value = allocator->Alloc(className, sizeof(typename ReturnType<T>::type));
			return value;
		}
	};

	template <class T>
	struct UniqueType {
		static Unique Get() {
			static IUniqueAllocator* allocator = GetGlobalUniqueAllocator();
#if defined(_MSC_VER) && _MSC_VER <= 1200
			static Unique unique = UniqueTypeEx<std::remove_reference<T>::type>::Get(allocator);
#else
			static Unique unique = UniqueTypeEx<typename std::remove_reference<T>::type>::Get(allocator);
#endif
			return unique;
		}

		static Unique Get(IUniqueAllocator* allocator) {
#if defined(_MSC_VER) && _MSC_VER <= 1200
			return UniqueTypeEx<std::remove_reference<T>::type>::Get(allocator);
#else
			return UniqueTypeEx<typename std::remove_reference<T>::type>::Get(allocator);
#endif
		}
	};


	class IReflect;
	class IIterator;

	// Reflectee
	class IReflectObject : public TObject<IReflect> {
	public:
		IReflectObject();
		// General object construction
		// We can initialize an IReflectObject with anything, and get a stub IReflectObject
		//     that IsBasicObject() == true && IsIterator() == false
		// It's useful when writing templates dealing with various pod types, see IStreamBase for more details
		template <class T>
		IReflectObject(const T& t) {}

		virtual ~IReflectObject();
		virtual TObject<IReflect>& operator () (IReflect& reflect) override;
		virtual const TObject<IReflect>& operator () (IReflect& reflect) const; // Just forward to non-const one 
		virtual bool IsBasicObject() const; // returns true, any sub-classes should return false;
		virtual bool IsIterator() const; // only for IIterators
		virtual Unique GetUnique() const;
		virtual IReflectObject* Clone() const;
		virtual String ToString() const;

		// Query interface, may be slow.
		// Same purpose as dynamic_cast<>
		template <class T>
		static T* QueryInterfaceEx(IReflectObject* object, Unique unique, UniqueType<T> target) {
			if (unique == target.Get()) return reinterpret_cast<T*>(object);

			const std::vector<std::pair<IUniqueInfo*, size_t> >& vec = unique.info->interfaces;
			IUniqueInfo* targetInfo = target.Get().info;
			std::vector<std::pair<IUniqueInfo*, size_t> >::const_iterator it = std::binary_find(vec.begin(), vec.end(), std::make_pair(targetInfo, (size_t)0), &IUniqueInfo::CompareInterface);

			// We do not support virtual inheritance
			return it != vec.end() ? reinterpret_cast<T*>((uint8_t*)object + it->second) : nullptr;
		}

		template <class T>
		T* QueryInterface(UniqueType<T> t) {
			return QueryInterfaceEx(this, GetUnique(), t);
		}

		template <class T>
		const T* QueryInterface(UniqueType<T> t) const {
			return QueryInterfaceEx(const_cast<IReflectObject*>(this), GetUnique(), t);
		}

		virtual IUniqueAllocator* GetUniqueAllocator() const;
		virtual void ReleaseObject(); // call delete this by default.
		virtual void FinalDestroy();

		// These two functions are work-arounds for gcc compiler.
		static const IReflectObject& TransformReflectObject(IReflectObject& t) {
			return t;
		}

		static const IReflectObject& TransformReflectObject(const IReflectObject& t) {
			static const IReflectObject s;
			return t.IsBasicObject() ? s : t;
		}
		
		// Not recommended for frequently calls
		template <class T>
		T* Inspect(UniqueType<T> unique, const String& key = "") {
			static Unique u = UniqueType<T>::Get();
			return reinterpret_cast<T*>(InspectEx(u, key));
		}

		virtual void* InspectEx(Unique unique, const String& key);
		virtual bool operator >> (IStreamBase& stream) const;
		virtual bool operator << (IStreamBase& stream);
	};

	// Non-pods base object
	class IReflectObjectComplex : public IReflectObject {
	public:
		virtual bool IsBasicObject() const;
		virtual TObject<IReflect>& operator () (IReflect& reflect) override;
	};

	// IIterator is for all iterators, usually created by IterateVector
	class IIterator : public IReflectObject {
	public:
		IIterator();
		virtual ~IIterator();
		virtual bool IsBasicObject() const;
		virtual bool IsIterator() const;
		virtual IIterator* New() const = 0;
		virtual void Attach(void* base) = 0;
		virtual void* GetHost() const = 0;
		virtual void Initialize(size_t count) = 0;
		virtual size_t GetTotalCount() const = 0;
		virtual void* Get() = 0;
		virtual const IReflectObject& GetPrototype() const = 0;
		virtual Unique GetPrototypeUnique() const = 0;
		virtual Unique GetPrototypeReferenceUnique() const = 0;
		virtual bool IsLayoutLinear() const = 0;
		virtual bool IsLayoutPinned() const = 0;
		virtual bool Next() = 0;
	};

	// IIterator for vector arrays
	template <class T>
	class VectorIterator : public IIterator {
	public:
#if defined(_MSC_VER) && _MSC_VER <= 1200
		typedef T::value_type value_type;
#else
		typedef typename T::value_type value_type;
#endif
		VectorIterator(T* p) : base(p), i(0) {}
		virtual void Initialize(size_t c) {
			assert(base != nullptr);
			base->resize(c);
		}

		virtual size_t GetTotalCount() const override {
			assert(base != nullptr);
			return base->size();
		}

		virtual Unique GetPrototypeUnique() const override {
			return UniqueType<value_type>::Get();
		}

		virtual Unique GetPrototypeReferenceUnique() const override {
#if defined(_MSC_VER) && _MSC_VER <= 1200
			return UniqueType<std::remove_pointer<value_type>::type>::Get();
#else
			return UniqueType<typename std::remove_pointer<value_type>::type>::Get();
#endif
		}

		virtual const IReflectObject& GetPrototype() const override {
			static const value_type t = value_type();
			static const IReflectObject& object = IReflectObject::TransformReflectObject(t);
			return object;
		}

		virtual void* Get() override {
			assert(i != 0);
			return &(*base)[i - 1];
		}

		virtual bool Next() override {
			if (i >= (*base).size()) {
				return false;
			}

			i++;
			return true;
		}

		virtual IIterator* New() const override {
			return new VectorIterator(base);
		}

		virtual void Attach(void* p) override {
			base = reinterpret_cast<T*>(p);
			i = 0;
		}

		virtual bool IsLayoutLinear() const override {
			return true;
		}

		virtual bool IsLayoutPinned() const override {
			return false;
		}

		virtual void* GetHost() const override {
			return base;
		}

		virtual Unique GetUnique() const override {
			typedef VectorIterator<T> Class;
			return UniqueType<Class>::Get();
		}

	private:
		T* base;
		size_t i;
	};

	// Powerful meta annotation support
	// We can attach more optional customized data (metadata) when reflecting a property/method/class.
	// Such as serialization state, script binding state, event doc string.
	// These metadata are protocols between reflector and reflectees.
	//

	class MetaNodeBase : public IReflectObjectComplex {
	public:
		virtual TObject<IReflect>& operator () (IReflect& reflect) {
			return *this;
		}
	};

	// MetaChain is often constructed by cascaded [][] operators
	// So they are usually temporary allocated objects
	// Be aware of thier lifetime!
	class MetaChainBase : public IReflectObjectComplex {
	public:
		virtual const MetaChainBase* GetNext() const {
			return nullptr;
		}

		virtual const MetaNodeBase* GetNode() const {
			return nullptr;
		}

		virtual const MetaNodeBase* GetRawNode() const {
			return nullptr;
		}

	private:
		virtual TObject<IReflect>& operator () (IReflect& reflect) {
			assert(false);
			return *this;
		}
	};

	template <class T, class D, class K, class Base>
	class MetaChain : public MetaChainBase {
	public:
		MetaChain(const K& k, const Base& b) : chainNode(k), rawChainNodePtr(&k), base(b) {}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		template <class N>
		inline MetaChain<T, D, N::Type, MetaChain<T, D, K, Base> > operator [] (const N& t) const {
			return MetaChain<T, D, N::Type, MetaChain<T, D, K, Base> >(t.FilterField(GetObject(), GetMember()), *this);
		}
#else
		template <class N>
		inline MetaChain<T, D, typename N::template RealType<T, D>::Type, MetaChain<T, D, K, Base> > operator [] (const N& t) const {
			return MetaChain<T, D, typename N::template RealType<T, D>::Type, MetaChain<T, D, K, Base> >(t.FilterField(GetObject(), GetMember()), *this);
		}
#endif

		inline bool IsChain() const {
			return true;
		}

		virtual const MetaChainBase* GetNext() const {
			return base.IsChain() ? &base : nullptr;
		}

		virtual const MetaNodeBase* GetNode() const {
			return &chainNode;
		}

		virtual const MetaNodeBase* GetRawNode() const {
			return rawChainNodePtr;
		}

		inline void operator * () const {
			Finish(this);
		}

		void Finish(const MetaChainBase* head) const {
			(const_cast<Base&>(base)).Finish(head);
		}

		inline T* GetObject() const {
			return base.GetObject();
		}

		inline D* GetMember() const {
			return base.GetMember();
		}

		K chainNode;
		const K* rawChainNodePtr;
		const Base& base;
	};

	// Reflector
	class IReflect {
	public:
		IReflect(bool reflectProperty = true, bool reflectMethod = false, bool reflectInterface = true, bool reflectEnum = false);
		virtual ~IReflect();

		bool IsReflectProperty() const;
		bool IsReflectMethod() const;
		bool IsReflectInterface() const;
		bool IsReflectEnum() const;

		void RegisterBuiltinTypes(bool useStdintType = false);
		struct Param {
			Param(Unique t = UniqueType<Void>::Get(), const String& n = "") : type(t), name(n) {}
			operator Unique () const {
				return type;
			}

			Unique type;
			String name;
		};

		// static size_t GetUniqueLength(Unique id);
		template <class T>
		inline void OnEnum(T& t, const char* name, const MetaChainBase* meta) {
			static Unique u = UniqueType<T>::Get();
			Enum(safe_cast<size_t>(t), u, name, meta);
		}

		template <class T>
		inline void OnClass(T& t, const char* name, const char* path, const MetaChainBase* meta) {
			static Unique u = UniqueType<T>::Get();
			Class(t, u, name, path, meta);
		}

		template <class T>
		inline void OnProperty(const T& t, const char* name, void* base, const MetaChainBase* meta) {
			static Unique u = UniqueType<T>::Get();
#if defined(_MSC_VER) && _MSC_VER <= 1200
			static Unique ur = UniqueType<std::remove_pointer<T>::type>::Get();
#else
			static Unique ur = UniqueType<typename std::remove_pointer<T>::type>::Get();
#endif
			ForwardProperty(const_cast<T&>(t), u, ur, name, base, (void*)&t, meta);
		}

		inline void ForwardProperty(const IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
			Property(const_cast<IReflectObject&>(s), typeID, refTypeID, name, base, ptr, meta);
		}

#if (defined(_MSC_VER) && _MSC_VER < 1800) || defined(COMPATIBLE_PROXY)
		// T == TWrapper<...>
		template <class R, class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
		inline void OnMethod(const TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P>& t, const char* name, const MetaChainBase* meta) {
			static Unique params[] = {
				UniqueType<A>::Get(),
				UniqueType<B>::Get(),
				UniqueType<C>::Get(),
				UniqueType<D>::Get(),
				UniqueType<E>::Get(),
				UniqueType<F>::Get(),
				UniqueType<G>::Get(),
				UniqueType<H>::Get(),
				UniqueType<I>::Get(),
				UniqueType<J>::Get(),
				UniqueType<K>::Get(),
				UniqueType<L>::Get(),
				UniqueType<M>::Get(),
				UniqueType<N>::Get(),
				UniqueType<O>::Get(),
				UniqueType<P>::Get()
			};

			static Param retValue = UniqueType<R>::Get();
			std::vector<Param> p;
			for (size_t i = 0; i < sizeof(params) / sizeof(params[0]) && (!(params[i] == UniqueType<Void>::Get())); i++) {
				p.emplace_back(Param(params[i]));
			}

			static Unique unique = UniqueType<TWrapper<R, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P> >::Get();
			Method(unique, name, reinterpret_cast<const TProxy<>*>(&t.GetProxy()), retValue, p, meta);
		}
#else
		template <typename R, typename... Args>
		inline void OnMethod(const TWrapper<R, Args...>& t, const char* name, const MetaChainBase* meta) {
			std::vector<Param> params;
			ParseParams(params, t);
			static Unique u = UniqueType<R>::Get();
			static Param retValue(u);
			static Unique unique = UniqueType<TWrapper<R, Args...> >::Get();
			Method(unique, name, reinterpret_cast<const TProxy<>*>(&t.GetProxy()), retValue, params, meta);
		}

		template <typename R, typename V, typename... Args>
		inline void ParseParams(std::vector<Param>& params, const TWrapper<R, V, Args...>&) {
			static Unique u = UniqueType<V>::Get();
			params.emplace_back(Param(u));
			ParseParams(params, TWrapper<R, Args...>());
		}

		template <typename R>
		inline void ParseParams(std::vector<Param>& params, const TWrapper<R>&) {}
#endif

		virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta);
		virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta);
		virtual void Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta);
		virtual void Enum(size_t value, Unique id, const char* name, const MetaChainBase* meta);

	private:
		bool isReflectProperty;
		bool isReflectMethod;
		bool isReflectInterface;
		bool isReflectEnum;
	};

	// Here are some sample Meta classes

	class MetaNote : public MetaNodeBase {
	public:
		MetaNote(const String& v);
		MetaNote operator = (const String& value);

		template <class T, class D>
		inline const MetaNote& FilterField(T* t, D* d) const {
			return *this; // do nothing
		}

		template <class T, class D>
		struct RealType {
			typedef MetaNote Type;
		};

		typedef MetaNote Type;

		virtual TObject<IReflect>& operator () (IReflect& reflect) override;
		const String& value;
	};

	extern MetaNote Note;

	template <class T, class D>
	class WriterBase : public MetaChainBase {
	public:
		WriterBase(IReflect& r, T* p, D* v, const char* n) : reflect(r), object(p), member(v), name(n) {}
		virtual ~WriterBase() {}

		inline bool IsChain() const { return false; }

		virtual void Finish(const MetaChainBase* head) {}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		template <class N>
		inline MetaChain<T, D, N::Type, WriterBase<T, D> > operator [] (const N& t) const {
			return MetaChain<T, D, N::Type, WriterBase<T, D> >(t.FilterField(object, member), *this);
		}

#else
		template <class N>
		inline MetaChain<T, D, typename N::template RealType<T, D>::Type, WriterBase<T, D> > operator [] (const N& t) const {
			return MetaChain<T, D, typename N::template RealType<T, D>::Type, WriterBase<T, D> >(t.FilterField(object, member), *this);
		}

#endif

		inline T* GetObject() const {
			return object;
		}

		inline D* GetMember() const {
			return member;
		}

		IReflect& reflect;
		T* object;
		D* member;
		const char* name;
	};

	template <class T>
	struct VectorIteratorHelper {
		template <class D>
		static VectorIterator<D> Transform(D* d) {
			return VectorIterator<D>(d);
		}
	};

	template <>
	struct VectorIteratorHelper<std::false_type> {
		template <class D>
		static D& Transform(D* d) {
			return *d;
		}
	};

	template <class T, class D>
	class PropertyWriter : public WriterBase<T, D> {
	public:
		PropertyWriter(IReflect& r, T* p, D* v, const char* n) : WriterBase<T, D>(r, p, v, n) {}
		~PropertyWriter() {}

		void operator * () {
			Finish(nullptr);
		}

		virtual void Finish(const MetaChainBase* head) {
#if defined(_MSC_VER) && _MSC_VER <= 1200
			WriterBase<T, D>::reflect.OnProperty(VectorIteratorHelper<std::is_vector<std::decay<D>::type>::type>::Transform(WriterBase<T, D>::member), WriterBase<T, D>::name, WriterBase<T, D>::object, head == this ? nullptr : head);
#else
			WriterBase<T, D>::reflect.OnProperty(VectorIteratorHelper<typename std::is_vector<typename std::decay<D>::type>::type>::Transform(WriterBase<T, D>::member), WriterBase<T, D>::name, WriterBase<T, D>::object, head == this ? nullptr : head);
#endif
		}
	};

	template <class T, class D>
	PropertyWriter<T, typename std::remove_shared<D>::type> CreatePropertyWriter(IReflect& r, T* p, const D& v, const char* name) {
#if defined(_MSC_VER) && _MSC_VER <= 1200
		return PropertyWriter<T, std::remove_shared<D>::type>(r, p, (std::remove_shared<D>::type*)const_cast<D*>(&v), name);
#else
		return PropertyWriter<T, typename std::remove_shared<D>::type>(r, p, (typename std::remove_shared<D>::type*)const_cast<D*>(&v), name);
#endif
	}

	template <class T, class D>
	class MethodWriter : public WriterBase<T, D> {
	public:
		MethodWriter(IReflect& r, T* p, D* v, const char* n) : WriterBase<T, D>(r, p, v, n) {}
		virtual ~MethodWriter() {}

		void operator * () {
			Finish(nullptr);
		}

		virtual void Finish(const MetaChainBase* head) {
			WriterBase<T, D>::reflect.OnMethod(Wrap(WriterBase<T, D>::object, *WriterBase<T, D>::member), WriterBase<T, D>::name, head == this ? nullptr : head);
		}
	};

	template <class T, class D>
	MethodWriter<T, D> CreateMethodWriter(IReflect& r, T* p, const D& v, const char* name) {
		return MethodWriter<T, D>(r, p, const_cast<D*>(&v), name);
	}

	template <class T>
	class ClassWriter : public WriterBase<T, const char>  {
	public:
		ClassWriter(IReflect& r, T* p, const char* l) : WriterBase<T, const char>(r, p, l, "Class") {}
		virtual ~ClassWriter() {}

		void operator * () {
			Finish(nullptr);
		}

		virtual void Finish(const MetaChainBase* head) {
			WriterBase<T, const char>::reflect.OnClass(*WriterBase<T, const char>::object, UniqueType<T>::Get()->typeName.c_str(), WriterBase<T, const char>::member, head);
		}
	};

	template <class T>
	ClassWriter<T> CreateClassWriter(IReflect& r, T* v, const char* line) {
		return ClassWriter<T>(r, v, line);
	}

	template <class T>
	class EnumWriter : public WriterBase<T, T>  {
	public:
		EnumWriter(IReflect& r, T v, const char* n) : value(v), WriterBase<T, T>(r, &value, &value, n) {}
		virtual ~EnumWriter() {}

		void operator * () {
			Finish(nullptr);
		}

		virtual void Finish(const MetaChainBase* head) {
			WriterBase<T, T>::reflect.OnEnum(value, WriterBase<T, T>::name, head);
		}

	private:
		T value;
	};

	template <class T>
	EnumWriter<T> CreateEnumWriter(IReflect& r, T v, const char* name) {
		return EnumWriter<T>(r, v, name);
	}

	template <class T>
	struct Creatable {
		Creatable() {
			static Unique u = UniqueType<T>::Get();
			u->creator = &Creatable::Create;
		}
		static void Init() {
			static Creatable<T> theInstance;
		}
		static IReflectObject* Create() {
			return new T();
		}
	};

	class MetaConstructable : public MetaNodeBase {
	public:
		template <class T, class D>
		inline const MetaConstructable& FilterField(T* t, D* d) const {
			static Creatable<T> theClass;
			return *this; // do nothing
		}

		template <class T, class D>
		struct RealType {
			typedef MetaConstructable Type;
		};

		typedef MetaConstructable Type;
		virtual Unique GetUnique() const override;
	};

	extern MetaConstructable Constructable;

	template <class T, class P>
	class RegisterInterface {
	public:
		RegisterInterface() {
			static Unique t = UniqueType<T>::Get();
			static Unique p = UniqueType<P>::Get();

			// We do not support virtual inheritance
			// Just gen offset from T to P
			const T* object = (const T*)0x1000;
			const P* convert = static_cast<const P*>(object);
			size_t offset = (uint8_t*)convert - (uint8_t*)object;

			t->interfaces.reserve(t->interfaces.size() + p->interfaces.size() + 1);
			std::binary_insert(t->interfaces, std::make_pair(p.info, offset), &IUniqueInfo::CompareInterface);
			// merge casts
			for (size_t k = 0; k < p->interfaces.size(); k++) {
				std::pair<IUniqueInfo*, size_t> v = p->interfaces[k];
				std::binary_insert(t->interfaces, std::make_pair(v.first, v.second + offset), &IUniqueInfo::CompareInterface);
			}
		}
	};

	class IUniversalInterface {
	public:
 		// Make MetaInterface happy.
		TObject<IReflect>& operator () (IReflect& reflect) {
			static IReflectObject dummy;
			return dummy;
		}
	};

	template <class P>
	class MetaInterface : public MetaNodeBase {
	public:
		P& object;
		class DummyReflect : public IReflect {
		public:
			DummyReflect() : IReflect(false, false) {}
		};

		MetaInterface(P& obj) : object(obj) {
			static DummyReflect dummyReflect;
			object.P::operator () (dummyReflect);
		}

		template <class T, class D>
		inline const MetaInterface<P>& FilterField(T* t, D* d) const {
			// class T is Interfaced form class P
			static TStaticInitializer<RegisterInterface<T, P> > reg;
			
			return *this; // do nothing
		}

		template <class T, class D>
		struct RealType {
			typedef MetaInterface<P> Type;
		};

		typedef MetaInterface<P> Type;

		virtual TObject<IReflect>& operator () (IReflect& reflect) {
			return object.P::operator () (reflect);
		}
	};

	template <class T>
	class BuiltinTypeWrapper : public IReflectObject {};

#define ReflectBuiltinSubtype(v) \
	{ static BuiltinTypeWrapper<v> proto; reflect.Class(proto, UniqueType<v>::Get(), #v, "C++", nullptr); }

#define ReflectBuiltinType(v) \
	ReflectBuiltinSubtype(v); \
	ReflectBuiltinSubtype(const v); \
	ReflectBuiltinSubtype(v&); \
	ReflectBuiltinSubtype(const v&); \
	ReflectBuiltinSubtype(v*); \
	ReflectBuiltinSubtype(const v*); \
	ReflectBuiltinSubtype(v*&); \
	ReflectBuiltinSubtype(const v*&); \

#define ReflectInterface(c) \
	MetaInterface<c>(*this)

#define ReflectClass(v) \
	*CreateClassWriter(reflect, static_cast<v*>(this), __FILE__)

#define ReflectEnum(v) \
	*CreateEnumWriter(reflect, v, #v);

#define ReflectProperty(v) \
	*CreatePropertyWriter(reflect, this, v, #v)

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define ReflectMethod(v) \
	*CreateMethodWriter(reflect, this, &Class::v, #v)
#else
#define ReflectMethod(v) \
	*CreateMethodWriter(reflect, this, &std::remove_reference<decltype(*this)>::type::v, #v)
#endif

	class Inspector : public IReflect {
	public:
		Inspector(const IReflectObject& r);

		template <class T>
		T* operator [](UniqueType<T> ut) {
			static Unique u = UniqueType<T>::Get();
			return reinterpret_cast<T*>(Find(u, ut));
		}

		virtual void* Find(Unique unique, const String& key) const;
		virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta);
		virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta);

	private:
		std::map<Unique, std::map<String, void*> > entries;
	};

	class MetaRuntime : public MetaNodeBase {
	public:
		template <class T, class D>
		inline const MetaRuntime& FilterField(T* t, D* d) const {
			return *this; // do nothing
		}

		template <class T, class D>
		struct RealType {
			typedef MetaRuntime Type;
		};

		typedef MetaRuntime Type;
		virtual TObject<IReflect>& operator () (IReflect& reflect) override;
	};

	extern MetaRuntime Runtime;

	class MetaVoid : public MetaNodeBase {
	public:
		template <class T, class D>
		inline const MetaVoid& FilterField(T* t, D* d) const {
			return *this; // do nothing
		}

		template <class T, class D>
		struct RealType {
			typedef MetaVoid Type;
		};

		typedef MetaVoid Type;
		virtual Unique GetUnique() const override;
	};

	// Helper class for implement rtti by simple inheriance.
	template <class T, class Base, class MetaOption = MetaVoid>
	class TReflected : public Base {
	protected:
#if defined(_MSC_VER) && _MSC_VER <= 1200
		TReflected() {}
		template <class A>
		TReflected(A& a) : Base(a) {}
		template <class A, class B>
		TReflected(A& a, B& b) : Base(a, b) {}
		template <class A, class B, class C>
		TReflected(A& a, B& b, C& c) : Base(a, b, c) {}
		template <class A, class B, class C, class D>
		TReflected(A& a, B& b, C& c, D& d) : Base(a, b, c, d) {}
		template <class A, class B, class C, class D, class E>
		TReflected(A& a, B& b, C& c, D& d, E& e) : Base(a, b, c, d, e) {}
		template <class A, class B, class C, class D, class E, class F>
		TReflected(A& a, B& b, C& c, D& d, E& e, F& f) : Base(a, b, c, d, e, f) {}
		template <class A, class B, class C, class D, class E, class F, class G>
		TReflected(A& a, B& b, C& c, D& d, E& e, F& f, G& g) : Base(a, b, c, d, e, f, g) {}
		template <class A, class B, class C, class D, class E, class F, class G, class H>
		TReflected(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h) : Base(a, b, c, d, e, f, g, h) {}
#else
		template <typename... Args>
		TReflected(Args&&... args) : Base(std::forward<Args>(args)...) {}
#endif
	public:
		typedef T Class;
		typedef TReflected BaseClass;
		typedef Base NativeBaseClass;

		virtual TObject<IReflect>& operator () (IReflect& reflect) override {
			static MetaOption option;
			ReflectClass(Class)[ReflectInterface(NativeBaseClass)][option];
			return *this;
		}

		virtual Unique GetUnique() const override {
			static Unique unique = IReflectObject::GetUnique();
			return unique;
		}
	};
}

#endif // __IREFLECT_H__
