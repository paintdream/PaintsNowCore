#include "IScript.h"
#include "../Template/TBuffer.h"

namespace PaintsNow {
	IScript::MetaLibrary ScriptLibrary;
	IScript::MetaMethod ScriptMethod;
	IScript::MetaVariable ScriptVariable;
	IScript::Request::TableStart begintable;
	IScript::Request::TableEnd endtable;
	IScript::Request::ArrayStart beginarray;
	IScript::Request::ArrayEnd endarray;
	IScript::Request::Key key("");
	IScript::Request::Nil nil;
	IScript::Request::Global global;
	IScript::Request::Ref ref;
	IScript::Request::Sync sync;
	IScript::Request::Deferred deferred;
}

using namespace PaintsNow;

IScript::Object::Object() {}

IScript::Object::~Object() {
	// printf("OBJECT DELETED %p\n", this);
}

void IScript::Object::ScriptUninitialize(Request& request) {
	ReleaseObject();
}

void IScript::Object::ScriptInitialize(Request& request) {}

IScript::Library::Library() {}

IScript::Library::~Library() {}

void IScript::Library::TickDevice(IDevice& device) {}

void IScript::Request::QueryInterface(const TWrapper<void, IScript::Request&, IReflectObject&, const Request::Ref&>& callback, IReflectObject& target, const Request::Ref& g) {
	// exports no interfaces by default
	callback(*this, target, g);
}

IScript::MetaLibrary::MetaLibrary(const String& n) : name(n) {}

TObject<IReflect>& IScript::MetaLibrary::operator () (IReflect& reflect) {
	BaseClass::operator () (reflect);
	if (reflect.IsReflectProperty()) {
		ReflectProperty(name);
	}

	return *this;
}

IScript::MetaLibrary IScript::MetaLibrary::operator = (const String& value) {
	return MetaLibrary(value);
}

template <bool init>
class Boostrapper : public IReflect {
public:
	Boostrapper() : IReflect(true, false) {}
	~Boostrapper() {
		if (!init) {
			for (size_t i = libraries.size(); i > 0; i--) {
				libraries[i - 1]->Uninitialize();
			}
		}
	}

	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		static Unique typedBaseType = UniqueType<IScript::MetaLibrary>::Get();
		
		if (!s.IsBasicObject() && s.QueryInterface(UniqueType<IScript::Library>()) != nullptr) {
			for (const MetaChainBase* t = meta; t != nullptr; t = t->GetNext()) {
				const MetaNodeBase* node = t->GetNode();
				if (!node->IsBasicObject() && node->GetUnique() == typedBaseType) {
					const IScript::MetaLibrary* entry = static_cast<const IScript::MetaLibrary*>(node);
					IScript::Library& lib = static_cast<IScript::Library&>(s);
					if (init) {
						lib.Initialize();
					} else {
						libraries.emplace_back(&lib);
					}
				}
			}
		}
	}

	std::vector<IScript::Library*> libraries;
};

void IScript::Library::Initialize() {
	Boostrapper<true> bootstrapper;
	(*this)(bootstrapper);
}

void IScript::Library::Uninitialize() {
	Boostrapper<false> bootstrapper;
	(*this)(bootstrapper);
}

class Registar : public IReflect {
public:
	Registar(IScript::Request& req) : IReflect(true, true), request(req) {}

	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		static Unique typedBaseType = UniqueType<IScript::MetaLibrary>::Get();
		
		if (!s.IsBasicObject() && s.QueryInterface(UniqueType<IScript::Library>()) != nullptr) {
			for (const MetaChainBase* t = meta; t != nullptr; t = t->GetNext()) {
				const MetaNodeBase* node = t->GetNode();
				if (!node->IsBasicObject() && node->GetUnique() == typedBaseType) {
					const IScript::MetaLibrary* entry = static_cast<const IScript::MetaLibrary*>(node);
					String n = entry->name.empty() ? name : entry->name;
					IScript::Library& lib = static_cast<IScript::Library&>(s);
					request << key(n) << lib;
				}
			}
		}
	}

	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {
		static Unique typedBaseType = UniqueType<IScript::MetaMethod::TypedBase>::Get();
		for (const MetaChainBase* t = meta; t != nullptr; t = t->GetNext()) {
			const MetaNodeBase* node = t->GetNode();
			if (!node->IsBasicObject() && node->GetUnique() == typedBaseType) {
				const IScript::MetaMethod::TypedBase* entry = static_cast<const IScript::MetaMethod::TypedBase*>(node);
				entry->Register(request, name);
			}
		}
	}

	IScript::Request& request;
};

void IScript::Library::Register(IScript::Request& request) {
	Registar reg(request);
	(*this)(reg);
}

void IScript::Library::Require(IScript::Request& request) {
	request << begintable;
	request << key("__delegate__") << this;
	Register(request);
	request << endtable;
}

void IScript::Library::ScriptInitialize(IScript::Request& request) {
}

void IScript::Library::ScriptUninitialize(IScript::Request& request) {
}

IScript::MetaVariable::MetaVariable(const String& k) : name(k) {}
IScript::MetaVariable::~MetaVariable() {}
IScript::MetaVariable IScript::MetaVariable::operator = (const String& k) {
	return MetaVariable(k);
}

template <bool read>
class Serializer : public IReflect {
public:
	Serializer(IScript::Request& req) : IReflect(true, false), request(req) {}
	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		static Unique typedBaseType = UniqueType<IScript::MetaVariable::TypedBase>::Get();
		for (const MetaChainBase* t = meta; t != nullptr; t = t->GetNext()) {
			const MetaNodeBase* node = t->GetNode();
			if (!node->IsBasicObject() && node->GetUnique() == typedBaseType) {
				IScript::MetaVariable::TypedBase* entry = const_cast<IScript::MetaVariable::TypedBase*>(static_cast<const IScript::MetaVariable::TypedBase*>(node));

				if (s.IsIterator()) {
					IScript::Request::TableStart ts;
					IIterator& it = static_cast<IIterator&>(s);

					if (read) {
						request >> key(entry->name.empty() ? name : entry->name) >> ts;
						it.Initialize((size_t)ts.count);
					} else {
						request << key(entry->name.empty() ? name : entry->name) << begintable;
					}

					if (it.GetPrototype().IsBasicObject()) {
						while (it.Next()) {
							void* ptr = it.Get();
							if (read) {
								entry->Read(request, false, name, ptr);
							} else {
								entry->Write(request, false, name, ptr);
							}
						}
					} else {
						while (it.Next()) {
							(*reinterpret_cast<IReflectObject*>(it.Get()))(*this);
						}
					}

					if (read) {
						request >> endtable;
					} else {
						request << endtable;
					}
				} else {
					if (read) {
						entry->Read(request, true, name, nullptr);
					} else {
						entry->Write(request, true, name, nullptr);
					}
				}
			}
		}
	}

	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {}


	IScript::Request& request;
	size_t itemCount;
};

IScript::IScript(IThread& api) : ISyncObject(api) {}

IScript::~IScript() {
}

IScript::Request::Request() : requestPool(nullptr) {}

void IScript::Request::SetRequestPool(IScript::RequestPool* p) {
	if (p != nullptr) {
		assert(GetScript() == &p->GetScript());
	}

	requestPool = p;
}

IScript::RequestPool* IScript::Request::GetRequestPool() {
	return requestPool;
}

IScript::Request::~Request() {}

void IScript::Request::DoLock() {
	GetScript()->DoLock();
}

void IScript::Request::UnLock() {
	GetScript()->UnLock();
}

IScript::Request::Ref::Ref(size_t i) : value(i) {}

IScript::Request::Key::Key(const String& k, IScript::Request::TYPE t) : type(t), key(k) {}
IScript::Request::Key IScript::Request::Key::operator () (const String& k) {
	return Key(k);
}


/*
IScript* IScript::Request::operator -> () {
	return GetRequest();
}*/

IScript::Request& IScript::Request::operator >> (Void*& t) {
	int64_t v = 0;
	(*this) >> v;
	t = (Void*)v;
	return *this;
}

IScript::Request& IScript::Request::operator << (Void* t) {
	(*this) << (int64_t)t;
	return *this;
}

#ifdef _MSC_VER
IScript::Request& IScript::Request::operator >> (long& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<long>(v);
	return *this;
}

IScript::Request& IScript::Request::operator >> (unsigned long& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<unsigned long>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (long t) {
	(*this) << (int64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator << (unsigned long t) {
	(*this) << (int64_t)t;
	return *this;
}
#endif

IScript::Request& IScript::Request::operator >> (int8_t& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<int8_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (int8_t t) {
	(*this) << (int64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (int16_t& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<int16_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (int16_t t) {
	(*this) << (int64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (int32_t& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<int32_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (int32_t t) {
	(*this) << (int64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (uint8_t& t) {
	uint64_t v = 0;
	(*this) >> v;
	t = safe_cast<uint8_t>(v);
	return *this;
}


IScript::Request& IScript::Request::operator << (uint8_t t) {
	(*this) << (uint64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (uint16_t& t) {
	uint64_t v = 0;
	(*this) >> v;
	t = safe_cast<uint16_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (uint16_t t) {
	(*this) << (uint64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (uint32_t& t) {
	uint64_t v = 0;
	(*this) >> v;
	t = safe_cast<uint32_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (uint32_t t) {
	(*this) << (uint64_t)t;
	return *this;
}

IScript::Request& IScript::Request::operator >> (uint64_t& t) {
	int64_t v = 0;
	(*this) >> v;
	t = safe_cast<uint64_t>(v);
	return *this;
}

IScript::Request& IScript::Request::operator << (uint64_t t) {
	(*this) << (int64_t)t;
	return *this;
}


IScript::Request& IScript::Request::operator << (Library& module) {
	module.Require(*this);
	return *this;
}


IScript::Request& IScript::Request::operator >> (Void&) {
	return *this;
}

IScript::Request& IScript::Request::operator << (const Void&) {
	return *this;
}

IScript::Request& IScript::Request::operator >> (float& value) {
	double db = 0;
	*this >> db;
	value = (float)db;
	return *this;
}

IScript::Request& IScript::Request::operator >> (IReflectObject& object) {
	assert(!object.IsBasicObject());
	*this >> begintable;
	Serializer<true> s(*this);
	object(s);
	*this >> endtable;

	return *this;
}

IScript::Request& IScript::Request::operator << (const IReflectObject& object) {
	assert(!object.IsBasicObject());
	*this << begintable;
	Serializer<false> s(*this);
	(const_cast<IReflectObject&>(object))(s);
	*this << endtable;

	return *this;
}

bool IScript::IsTypeCompatible(Request::TYPE target, Request::TYPE source) const {
	return target == source;
}

void IScript::SetErrorHandler(const TWrapper<void, Request&, const String&>& handler) {
	errorHandler = handler;
}

void IScript::SetDispatcher(const TWrapper<void, Request&, IHost*, size_t, const TWrapper<void, Request&>&>& disp) {
	dispatcher = disp;
}

const TWrapper<void, IScript::Request&, IHost*, size_t, const TWrapper<void, IScript::Request&>&>& IScript::GetDispatcher() const {
	return dispatcher;
}

void IScript::Request::Error(const String& msg) {
	const TWrapper<void, Request&, const String&>& handler = GetScript()->errorHandler;
	if (handler) {
		handler(*this, msg);
	}
}

/*
#include <Windows.h>

void IScript::DoLock() {
	DWORD id = ::GetCurrentThreadId();

	printf("Thread ID %d try to lock script\n", id);
	ISyncObject::DoLock();
	printf("Thread ID %d sucessfully locked script\n", id);
}

void IScript::UnLock() {
	DWORD id = ::GetCurrentThreadId();
	printf("Thread ID %d unlocked script\n", id);
	ISyncObject::UnLock();
}*/

void IScript::Reset() {
}

void IScript::Request::Key::SetKey(const char* k) {
	key = k;
}

const char* IScript::Request::Key::GetKey() const {
	return key.c_str();
}

IScript::Request::TYPE IScript::Request::Key::GetType() const {
	return type;
}

void IScript::Request::Key::SetType(IScript::Request::TYPE t) {
	type = t;
}


bool IScript::Request::AutoWrapperBase::IsSync() const {
	return false;
}

bool IScript::Request::Sync::IsSync() const {
	return true;
}

void IScript::Request::Sync::Execute(Request& request) const {
	assert(false);
}


IScript::Request::AutoWrapperBase* IScript::Request::Sync::Clone() const {
	return nullptr;
}

bool IScript::Request::Deferred::IsSync() const {
	return false;
}

void IScript::Request::Deferred::Execute(Request& request) const {}

IScript::Request::AutoWrapperBase* IScript::Request::Deferred::Clone() const {
	return nullptr;
}

IScript::MetaMethod::MetaMethod(const String& k) : key(k) {}
IScript::MetaMethod::~MetaMethod() {}

IScript::MetaMethod IScript::MetaMethod::operator = (const String& key) {
	return MetaMethod(key);
}

IScript::RequestPool::RequestPool(IScript& pscript, uint32_t psize) : script(pscript), size(psize) {
	requestCritical.store(0, std::memory_order_relaxed);
}

IScript::RequestPool::~RequestPool() {
	Clear();
}

IScript& IScript::RequestPool::GetScript() {
	return script;
}

void IScript::RequestPool::Clear() {
	SpinLock(requestCritical);
	std::stack<IScript::Request*> s;
	std::swap(s, requests);
	SpinUnLock(requestCritical);

	if (!s.empty()) {
		script.DoLock();
		while (!s.empty()) {
			IScript::Request* request = s.top();
			request->ReleaseObject();
			s.pop();
		}
		script.UnLock();
	}
}

IScript::Request* IScript::RequestPool::AllocateRequest() {
	IScript::Request* request = nullptr;
	SpinLock(requestCritical);
	if (!requests.empty()) {
		request = requests.top();
		assert(request->GetRequestPool() == this);
		requests.pop();
	}
	SpinUnLock(requestCritical);

	if (request == nullptr) {
		script.DoLock();
		request = script.NewRequest();
		request->SetRequestPool(this);
		script.UnLock();
	}

	return request;
}

void IScript::RequestPool::FreeRequest(IScript::Request* request) {
	SpinLock(requestCritical);
	if (requests.size() < size) {
		requests.push(request);
		request = nullptr;
	}
	SpinUnLock(requestCritical);

	if (request != nullptr) {
		script.DoLock();
		request->ReleaseObject();
		script.UnLock();
	}
}
