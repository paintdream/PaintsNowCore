#include "IReflect.h"
#include "IStreamBase.h"
#include "../Template/TBuffer.h"
#include <sstream>
using namespace PaintsNow;

String UniqueInfo::GetBriefName() const {
	String s = typeName;
	size_t k = 0;
	for (size_t i = 0, j = 0; i < typeName.size(); i++) {
		if (s[i] == ':') {
			k = j;
		} else {
			if (s[i] == '<' || s[i] == '>' || s[i] == ' ') j = i + 1;
			s[k++] = s[i];
		}
	}

	s[k] = '\0';
	return String(s.c_str(), k);
}

IReflectObject::IReflectObject() {}
IReflectObject::~IReflectObject() {}

void IReflectObject::ReleaseObject() {
	FinalDestroy();
}

void IReflectObject::FinalDestroy() {
	delete this;
}

bool IReflectObject::IsIterator() const {
	return false;
}

TObject<IReflect>& IReflectObject::operator () (IReflect& reflect) {
	assert(false); // not allowed. must override
	return *this;
}

bool IReflectObject::IsBasicObject() const {
	return true;
}

const TObject<IReflect>& IReflectObject::operator () (IReflect& reflect) const {
	const TObject<IReflect>& t = *this;
	(const_cast<TObject<IReflect>&>(t))(reflect);
	return *this;
}

IReflect::IReflect(bool property, bool method, bool inter, bool e) : isReflectProperty(property), isReflectMethod(method), isReflectInterface(inter), isReflectEnum(e) {}

IReflect::~IReflect() {}

bool IReflect::IsReflectProperty() const {
	return isReflectProperty;
}

bool IReflect::IsReflectMethod() const {
	return isReflectMethod;
}

bool IReflect::IsReflectInterface() const {
	return isReflectInterface;
}

bool IReflect::IsReflectEnum() const {
	return isReflectEnum;
}

class Generic : public IReflect {
public:
	Generic(const String& k, Unique u, IReflectObject& object) : IReflect(true, false), key(k), targetUnique(u), target(nullptr) {}

	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		if (typeID == targetUnique && (key.empty() || key == name)) {
			if (s.IsBasicObject()) {
				target = ptr;
			} else {
				target = &s;
			}
		}
	}

	void* GetTarget() const { return target; }
	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {}
	

private:
	const String& key;
	Unique targetUnique;
	void* target;
};

void* IReflectObject::InspectEx(Unique unique, const String& key) {
	Generic g(key, unique, *this);
	(*this)(g);

	return g.GetTarget();
}

Inspector::Inspector(const IReflectObject& r) : IReflect(true, false) {
	(const_cast<IReflectObject&>(r))(*this);
}

void Inspector::Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
	entries[typeID][name] = s.IsBasicObject() ? ptr : &s;
}

void Inspector::Method(Unique typeID, const char* name, const TProxy<>* p, const IReflect::Param& retValue, const std::vector<IReflect::Param>& params, const MetaChainBase* meta) {}

void* Inspector::Find(Unique unique, const String& filter) const {
	std::map<Unique, std::map<String, void*> >::const_iterator it = entries.find(unique);
	if (it != entries.end()) {
		const std::map<String, void*>& n = it->second;
		if (filter.empty()) {
			return n.empty() ? nullptr : n.begin()->second;
		} else {
			const std::map<String, void*>::const_iterator& p = n.find(filter);
			return p == n.end() ? nullptr : p->second;
		}
	} else {
		return nullptr;
	}
}

bool IReflectObject::operator << (IStreamBase& stream) {
	return stream.Read(const_cast<IReflectObject&>(*this), GetUnique(), const_cast<IReflectObject*>(this), sizeof(*this));
}

bool IReflectObject::operator >> (IStreamBase& stream) const {
	return stream.Write(const_cast<IReflectObject&>(*this), GetUnique(), const_cast<IReflectObject*>(this), sizeof(*this));
}

/*
size_t IReflect::GetUniqueLength(Unique id) {
	return *(reinterpret_cast<size_t*>(id));
}*/

bool IReflectObjectComplex::IsBasicObject() const {
	return false;
}

class ComputeMemoryUsage : public IReflect {
public:
	ComputeMemoryUsage(const IReflectObject& object) : size(object.GetUnique()->GetSize()) {
		(const_cast<IReflectObject&>(object))(*this);
	}

	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		static Unique uniqueString = UniqueType<String>::Get();
		static Unique uniqueBytes = UniqueType<Bytes>::Get();

		if (!s.IsBasicObject()) {
			if (s.IsIterator()) {
				IIterator& iterator = static_cast<IIterator&>(s);
				if (iterator.GetPrototype().IsBasicObject()) {
					size += iterator.GetPrototypeUnique()->GetSize() * iterator.GetTotalCount();
				} else {
					while (iterator.Next()) {
						IReflectObjectComplex* object = reinterpret_cast<IReflectObjectComplex*>(iterator.Get());
						size += object->ReportMemoryUsage();
					}
				}
			} else {
				size += static_cast<IReflectObjectComplex&>(s).ReportMemoryUsage() - typeID->GetSize();
			}
		} else if (typeID == uniqueBytes) {
			Bytes& bytes = *reinterpret_cast<Bytes*>(ptr);
			if (!bytes.IsStockStorage()) {
				size += bytes.GetSize();
			}
		} else if (typeID == uniqueString) {
			size += reinterpret_cast<String*>(ptr)->size();
		}

		/* else if (typeID != refTypeID) // We do not take pointers into consideration now. */
	}

	size_t size;
};

String IReflectObjectComplex::ToString() const {
	std::stringstream ss;
	ss << GetUnique()->GetName() << " (" << std::hex << (void*)this << " )";
	return ss.str();
}

size_t IReflectObjectComplex::ReportMemoryUsage() const {
	ComputeMemoryUsage compute(*this);
	return compute.size;
}

TObject<IReflect>& IReflectObjectComplex::operator () (IReflect& reflect) {
	ReflectClass(IReflectObjectComplex);

	return *this; // no operations
}


// IIterator
IIterator::IIterator() {}
IIterator::~IIterator() {}

String IIterator::ToString() const {
	std::stringstream ss;
	ss << "Collection<" << GetPrototypeUnique()->GetName() << "> (" << std::hex << (size_t)this << " ) [" << std::dec << GetTotalCount() << "]";
	return ss.str();
}

bool IIterator::IsBasicObject() const {
	return false;
}

bool IIterator::IsIterator() const {
	return true;
}

void IReflect::RegisterBuiltinTypes(bool useStdintType) {
	IReflect& reflect = *this;
	ReflectBuiltinType(String);
	ReflectBuiltinType(void*);

	if (useStdintType) {
		ReflectBuiltinType(size_t);
		ReflectBuiltinType(int8_t);
		ReflectBuiltinType(uint8_t);
		ReflectBuiltinType(int16_t);
		ReflectBuiltinType(uint16_t);
		ReflectBuiltinType(int32_t);
		ReflectBuiltinType(uint32_t);
		ReflectBuiltinType(long);
		ReflectBuiltinType(unsigned long);
	} else {
		ReflectBuiltinType(bool);
		ReflectBuiltinType(signed char);
		ReflectBuiltinType(char);
		ReflectBuiltinType(unsigned char);
		ReflectBuiltinType(signed int);
		ReflectBuiltinType(int);
		ReflectBuiltinType(unsigned int);
		ReflectBuiltinType(signed short);
		ReflectBuiltinType(short);
		ReflectBuiltinType(unsigned short);
		ReflectBuiltinType(signed long);
		ReflectBuiltinType(long);
		ReflectBuiltinType(unsigned long);
	}

	ReflectBuiltinType(int64_t);
	ReflectBuiltinType(uint64_t);
	ReflectBuiltinType(float);
	ReflectBuiltinType(double);

	ReflectBuiltinType(Int2);
	ReflectBuiltinType(Int3);
	ReflectBuiltinType(Int4);
	ReflectBuiltinType(Float2);
	ReflectBuiltinType(Float2Pair);
	ReflectBuiltinType(Float3);
	ReflectBuiltinType(Float3Pair);
	ReflectBuiltinType(Float4);
	ReflectBuiltinType(Float4Pair);
	ReflectBuiltinType(Double2);
	ReflectBuiltinType(Double2Pair);
	ReflectBuiltinType(Double3);
	ReflectBuiltinType(Double3Pair);
	ReflectBuiltinType(Double4);
	ReflectBuiltinType(Double4Pair);
	ReflectBuiltinType(MatrixFloat3x3);
	ReflectBuiltinType(MatrixFloat4x4);
}

void IReflect::Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {}

void IReflect::Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {}

void IReflect::Enum(size_t value, Unique id, const char* name, const MetaChainBase* meta) {}

void IReflect::Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta) {
	while (meta != nullptr) {
		MetaNodeBase* node = const_cast<MetaNodeBase*>(meta->GetNode());
		assert(node != nullptr);
		assert(!node->IsBasicObject());
		(*node)(*this);

		meta = meta->GetNext();
	}
}

class ReflectQueryType : public IReflect {
public:
	ReflectQueryType(IReflectObject& object);
	operator Unique () const {
		return type;
	}

public:
	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta);
	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta);
	virtual void Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta);

private:
	Unique type;
};

// ReflectQueryType

ReflectQueryType::ReflectQueryType(IReflectObject& object) : IReflect(false, false) {
	object(*this);
}

void ReflectQueryType::Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {}

void ReflectQueryType::Method(Unique typeID, const char* name, const TProxy<>* p, const IReflect::Param& retValue, const std::vector<IReflect::Param>& params, const MetaChainBase* meta) {}

void ReflectQueryType::Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta) {
	type = id;
}

Unique IReflectObject::GetUnique() const {
	ReflectQueryType query(const_cast<IReflectObject&>(*this));
	assert((Unique)query); // no class provided.
	return query;
}

IReflectObject* IReflectObject::Clone() const {
	assert(false); // not clonable by default
	return nullptr;
}

String IReflectObject::ToString() const {
	char str[32];
	sprintf(str, "0x%p", this);
	return str;
}

UniqueAllocator::UniqueAllocator() {
	critical.store(0, std::memory_order_relaxed);
}

UniqueInfo* UniqueAllocator::Create(const String& key, size_t size) {
	SpinLock(critical);

	std::unordered_map<String, UniqueInfo>::iterator it = mapType.find(key);
	UniqueInfo* ret = nullptr;
	if (it == mapType.end()) {
		UniqueInfo info;
		info.typeName = key;
		info.size = size;
		info.allocator = this;
		ret = &(mapType[key] = info);
	} else {
		ret = &(mapType[key]);
		if (size != 0) {
			ret->size = size;
		}
	}

	SpinUnLock(critical);
	return ret;
}


MetaNote::MetaNote(const String& v) : value(v) {}

MetaNote MetaNote::operator = (const String& value) {
	return MetaNote(value);
}

TObject<IReflect>& MetaNote::operator () (IReflect& reflect) {
	ReflectClass(MetaNote);
	if (reflect.IsReflectProperty()) {
		ReflectProperty(value);
	}

	return *this;
}

TObject<IReflect>& MetaRuntime::operator () (IReflect& reflect) {
	ReflectClass(MetaRuntime);
	return *this;
}

Unique MetaConstructable::GetUnique() const {
	return UniqueType<Void>::Get();
}

Unique MetaVoid::GetUnique() const {
	return UniqueType<Void>::Get();
}

namespace PaintsNow {
	UniqueAllocator& UniqueAllocator::GetInstance() {
		static UniqueAllocator theAllocator;
		return theAllocator;
	}

	MetaNote Note("");
	MetaConstructable Constructable;
	MetaRuntime Runtime;
}