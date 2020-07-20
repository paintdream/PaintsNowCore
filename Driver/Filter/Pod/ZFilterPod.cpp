#include "ZFilterPod.h"
#include "../../../Template/TObject.h"
#include "../../../Template/TBuffer.h"
#include <string>
#include <stack>
#include <cstdio>
#include <cassert>
#include <sstream>

extern "C" {
#include "Core/Pod.h"
}

using namespace PaintsNow;

class FilterPodImpl : public IStreamBase, public IReflect {
public:
	FilterPodImpl(IStreamBase& streamBase);
	virtual ~FilterPodImpl();

	virtual bool Read(void* p, size_t& len);
	virtual bool Write(const void* p, size_t& len);
	virtual bool Transfer(IStreamBase& stream, size_t& len);
	virtual bool WriteDummy(size_t& len);
	virtual void Flush();
	virtual bool Seek(IStreamBase::SEEK_OPTION option, long offset);

	// object writing/reading routine
	virtual bool Write(IReflectObject& a, Unique type, void* ptr, size_t length);
	virtual bool Read(IReflectObject& a, Unique type, void* ptr, size_t length);

	IStreamBase& GetBaseStreamImpl();
	virtual IStreamBase& GetBaseStream();

public:
	// IReflect
	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta);
	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta);
	virtual void Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta);

	IReflectObjectComplex* CreateObjectFromMeta(const String& metaData);
	String GetMetaFromObject(IReflectObjectComplex& object);
	void OnSubClass(IReflectObject& s, const char* name);

	struct Type {
		Type();
		~Type();
		Pod* pod;
		IIterator* iterator;
		MetaStreamPersist* persister;
		MetaStreamPersist* subPersister;
	};

	const Type& GetRegisteredType(IReflectObject& s, Unique unique, const String& ext = "");
	void Register(IReflectObject& s);
	void ReadTree();
	void WriteTree();
	void SyncTree();

	void MovePointer(int64_t offset);
	int64_t GetPointer() const;

private:
	Type& NewType(Unique id, const String& name, const String& ext, bool& created);
	std::map<std::pair<Unique, String>, Type> mapTypes;
	Type* current;
	PodRoot* root;
	PodRoot* readRoot;
	int64_t offset;

protected:
	IStreamBase& stream;
};

FilterPodImpl::Type::Type() : pod(nullptr), iterator(nullptr), persister(nullptr), subPersister(nullptr) {}
FilterPodImpl::Type::~Type() {
	if (persister != nullptr) {
		persister->ReleaseObject();
	}
}

IStreamBase& FilterPodImpl::GetBaseStream() {
	return GetBaseStreamImpl();
}

// Pod adapters

static void* StringLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	*iterator = nullptr;
	String& str = *reinterpret_cast<String*>(base);
	if (*count != (PodSize)-1) { // write
		str.resize((size_t)*count);
	} else {
		*count = (PodSize)(str.size());
	}

	return const_cast<char*>(str.data());
}

static void* BufferLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	*iterator = nullptr;
	Bytes& str = *reinterpret_cast<Bytes*>(base);
	if (*count != (PodSize)-1) { // write
		str.Resize((size_t)*count);
	} else {
		*count = (PodSize)(str.GetSize());
	}

	return str.GetData();
}

static void* PointerLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	assert(base != nullptr);
	assert(metaLength != nullptr);
	assert(dynamicType != nullptr);
	assert(context != nullptr);
	*iterator = nullptr;

	FilterPodImpl* filterPod = reinterpret_cast<FilterPodImpl*>(context);
	IReflectObjectComplex*& object = *reinterpret_cast<IReflectObjectComplex**>(base);
	bool read = *count != -1;
	*count = 1;

	if (read) {
		object = filterPod->CreateObjectFromMeta(String(metaData, *metaLength));
	} else if (object != nullptr) {
		String metaString = filterPod->GetMetaFromObject(*object);
		assert(!metaString.empty());
		if (metaString.empty()) return nullptr; // not supported
		assert(metaString.length() < MAX_META_LENGTH);
		memcpy(metaData, metaString.c_str(), metaString.length());
		*metaLength = (uint32_t)metaString.length();
	} else {
		// empty pointer
		*metaLength = 0;
	}

	// make object type registered
	if (object != nullptr) {
		*dynamicType = filterPod->GetRegisteredType(*object, object->GetUnique()).pod;
	}

	return object;
}

static void* InvalidLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	return nullptr;
}

static void* DirectLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	*count = 1;
	*iterator = nullptr;

	return base;
}

static void* StreamLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	*iterator = nullptr;
	FilterPodImpl::Type* type = (FilterPodImpl::Type*)locateContext;
	IIterator* it = type->iterator->New();
	it->Attach(base);
	if (*count != -1) {
		// read
		it->Initialize((size_t)*count);
	} else {
		// write
		*count = (PodSize)it->GetTotalCount();
	}

	// check linearness
	assert(type->persister == nullptr);
	if (type->subPersister == nullptr && it->IsLayoutLinear() && it->GetPrototype().IsBasicObject()) {
		// can copy directly
		void* obj = nullptr;
		if (it->Next()) {
			// get first element...
			obj = it->Get();
		}

		it->ReleaseObject();
		return obj;
	} else {
		*iterator = (void*)it;
		return nullptr; // before first one, returns nullptr.
	}
}

static void* StreamIterateHandler(void* locateContext, void** pointer, void* context) {
	IIterator* iterator = *(IIterator**)pointer;
	assert(iterator != nullptr);

	if (iterator->Next()) {
		return iterator->Get();
	} else {
		iterator->ReleaseObject();
		*pointer = nullptr;
		return nullptr;
	}
}

static void* CustomizedLocateHandler(void* locateContext, void** iterator, PodSize* count, void* base, char* metaData, uint32_t* metaLength, const Pod** dynamicType, void* context) {
	MetaStreamPersist* persist = reinterpret_cast<MetaStreamPersist*>(locateContext);
	FilterPodImpl* p = (FilterPodImpl*)context;
	StreamBaseMeasure s(p->GetBaseStreamImpl());
	*iterator = nullptr;

	// TODO: allow error cdoe
	if (*count != (PodSize)-1) { // write
		assert(*count == 1);
		persist->Read(s, base);
	} else {
		persist->Write(s, base);
		*count = 1;
	}

	p->MovePointer(s.transition);
	return nullptr;
}

static int StreamWriterFile(const PodStream* stream, const void* base, PodSize size, void* context) {
	FilterPodImpl* p = (FilterPodImpl*)context;
	IStreamBase& s = p->GetBaseStreamImpl();

	size_t len = (size_t)size;
	int ret = s.WriteBlock(base, len) ? POD_SUCCESS : POD_ERROR_STREAM;
	p->MovePointer(len);

	return ret;
}

static int StreamReaderFile(const PodStream* stream, void* base, PodSize size, void* context) {
	FilterPodImpl* p = (FilterPodImpl*)context;
	IStreamBase& s = p->GetBaseStreamImpl();
	size_t len = (size_t)size;
	int ret = s.ReadBlock(base, len) ? POD_SUCCESS : POD_ERROR_STREAM;

	p->MovePointer(len);
	assert(ret == POD_SUCCESS);
	return ret;
}

static PodSize StreamLocaterFile(const PodStream* stream, void* context) {
	FilterPodImpl* p = (FilterPodImpl*)context;
	// IStreamBase* s = p->GetBaseStreamImpl();

	return (PodSize)p->GetPointer();
}

static int StreamSeekerFile(const PodStream* stream, uint8_t direct, PodSize step, void* context) {
	FilterPodImpl* p = (FilterPodImpl*)context;
	IStreamBase& s = p->GetBaseStreamImpl();

	long t = direct ? (long)step : (long)-(int64_t)step;
	p->MovePointer(t);
	return s.Seek(IStreamBase::CUR, t) ? POD_SUCCESS : POD_ERROR_STREAM;
}

static PodStream streamAdapter = { StreamWriterFile, StreamReaderFile, StreamSeekerFile, StreamLocaterFile };

// currently we do not process methods, so set the 2nd option should be set to false
FilterPodImpl::FilterPodImpl(IStreamBase& streamBase) : stream(streamBase), IReflect(true, false), current(nullptr), offset(0) {
	SetEnvironment(streamBase.GetEnvironment());

	PodInit();
	root = PodCreateRoot();
	readRoot = PodCreateRoot();
}

void FilterPodImpl::MovePointer(int64_t delta) {
	offset += delta;
}

IStreamBase& FilterPodImpl::GetBaseStreamImpl() {
	return stream;
}

int64_t FilterPodImpl::GetPointer() const {
	return offset;
}

FilterPodImpl::~FilterPodImpl() {
	for (std::map<std::pair<Unique, String>, Type>::iterator it = mapTypes.begin(); it != mapTypes.end(); ++it) {
		IIterator* p = it->second.iterator;
		if (p != nullptr) {
			p->ReleaseObject();
		}

		// PodDelete(it->second.pod);
	}

	PodDeleteRoot(readRoot);
	PodDeleteRoot(root);
	PodExit();
}

void FilterPodImpl::Flush() {
	stream.Flush();
}

bool FilterPodImpl::Read(void* p, size_t& len) {
	assert(false);
	return stream.Read(p, len);
}

bool FilterPodImpl::Write(const void* p, size_t& len) {
	assert(false);
	return stream.WriteBlock(p, len);
}

bool FilterPodImpl::Transfer(IStreamBase& s, size_t& len) {
	assert(false);
	return stream.Transfer(s, len);
}

bool FilterPodImpl::Seek(SEEK_OPTION option, long offset) {
	assert(false);
	return stream.Seek(option, offset);
}

void FilterPodImpl::WriteTree() {
	PodWriteSpecRoot(root, &streamAdapter, this);
}

void FilterPodImpl::ReadTree() {
	PodParseSpecRoot(readRoot, &streamAdapter, this);
}

void FilterPodImpl::SyncTree() {
	PodSyncRoot(root, readRoot);
}

const FilterPodImpl::Type& FilterPodImpl::GetRegisteredType(IReflectObject& s, Unique unique, const String& ext) {
	std::map<std::pair<Unique, String>, Type>::iterator p = mapTypes.find(std::make_pair(unique, ext));
	if (p == mapTypes.end()) {
		// Not registered
		Register(s);

		return mapTypes[std::make_pair(unique, ext)];
	} else {
		assert(p->second.pod != nullptr);
		return p->second;
	}
}

bool FilterPodImpl::Write(IReflectObject& s, Unique unique, void* ptr, size_t length) {
	const Type& type = GetRegisteredType(s, unique);
	return PodWriteData(type.pod, &streamAdapter, ptr, this) == POD_SUCCESS;
}

bool FilterPodImpl::WriteDummy(size_t& len) {
	return stream.WriteDummy(len);
}

bool FilterPodImpl::Read(IReflectObject& s, Unique unique, void* ptr, size_t length) {
	GetRegisteredType(s, unique);
	return PodParseData(root, &streamAdapter, ptr, this) == POD_SUCCESS;
}

void FilterPodImpl::OnSubClass(IReflectObject& s, const char* name) {
	Type* save = current;
	current = nullptr; // if some class do not use ReflectClass before other Reflect macros, use nullptr to assert them
	s(*this);
	// now current is sub type ...
	assert(current != nullptr);
	current = save;
}

IReflectObjectComplex* FilterPodImpl::CreateObjectFromMeta(const String& metaData) {
	Unique unique(metaData);
	return (bool)unique && unique->IsCreatable() ? static_cast<IReflectObjectComplex*>(unique->Create()) : nullptr;
}

String FilterPodImpl::GetMetaFromObject(IReflectObjectComplex& object) {
	return object.GetUnique()->GetName();
}

void FilterPodImpl::Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
	if (current == nullptr)
		return;

	static Unique metaRuntime = UniqueType<MetaRuntime>::Get();
	// check serialization chain
	const MetaStreamPersist* customPersister = nullptr;
	while (meta != nullptr) {
		// runtime data can not be serialized
		if (meta->GetNode()->GetUnique() == metaRuntime) {
			return;
		} else if (customPersister == nullptr) {
			const MetaStreamPersist* persistHelper = meta->GetNode()->QueryInterface(UniqueType<MetaStreamPersist>());
			if (persistHelper != nullptr) {
				customPersister = persistHelper;
			}
		}

		meta = meta->GetNext();
	}

	String postfix;
	if (customPersister != nullptr) {
		postfix = String("@") + customPersister->GetUniqueName();
	}

	static const Unique uniqueString = UniqueType<String>::Get();
	static const Unique uniqueByteBuffer = UniqueType<Bytes>::Get();

	assert((const char*)ptr - (const char*)base != 0); // must have space for virtual table?
	// Process Attrib
	if (s.IsBasicObject()) {
		bool isByteBuffer = typeID == uniqueByteBuffer;
		bool isString = typeID == uniqueString;
		bool isPointer = typeID != refTypeID;
		if (isString || isByteBuffer || isPointer || customPersister != nullptr) {
			bool created = false;
			Type& type = NewType(typeID, typeID->GetName(), postfix, created);
			if (created) {
				if (customPersister != nullptr) {
					type.persister = static_cast<MetaStreamPersist*>(customPersister->Clone());
					PodSetHandler(type.pod, CustomizedLocateHandler, nullptr, type.persister);
				} else if (isString || isByteBuffer) {
					PodSetHandler(type.pod, isString ? StringLocateHandler : BufferLocateHandler, nullptr, (void*)&type);
					PodInsert(type.pod, (const uint8_t*)"buffer", 0, true, 0, 0, POD_TYPE(sizeof(uint8_t)));
				} else {
					PodSetHandler(type.pod, PointerLocateHandler, nullptr, (void*)this);
				}
			}

			PodInsert(current->pod, (const uint8_t*)name, (PodSize)((const char*)ptr - (const char*)base), 1, 0, isPointer && customPersister == nullptr ? 1 : 0, type.pod);
		} else {
			PodInsert(current->pod, (const uint8_t*)name, (PodSize)((const char*)ptr - (const char*)base), 0, 0, 0, POD_TYPE(typeID->GetSize()));
		}
	} else if (s.IsIterator()) {
		IIterator& iter = static_cast<IIterator&>(s);
		std::ostringstream os;
		os << name << "[" << (size_t)typeID.GetInfo() << "]";
		bool created = false;
		Type& type = NewType(typeID, os.str(), postfix, created);

		if (created) {
			type.iterator = (static_cast<IIterator&>(s)).New();
			const IReflectObject& prototype = iter.GetPrototype();
			Unique subUnique = iter.GetPrototypeUnique();
			if (prototype.IsBasicObject() && customPersister == nullptr) {
				if (subUnique != iter.GetPrototypeReferenceUnique()) {
					Type& subType = NewType(subUnique, subUnique->GetName(), postfix, created);
					PodInsert(type.pod, (const uint8_t*)"[]", 0, 0, 0, 1, subType.pod);
				} else {
					PodInsert(type.pod, (const uint8_t*)"[]", 0, 0, 0, 0, POD_TYPE(subUnique->GetSize()));
				}
			} else {
				Type& subType = NewType(subUnique, subUnique->GetName(), postfix, created);
				if (created) {
					if (customPersister != nullptr) {
						type.subPersister = subType.persister = static_cast<MetaStreamPersist*>(customPersister->Clone());
						PodSetHandler(subType.pod, CustomizedLocateHandler, nullptr, subType.persister);
					} else {
						OnSubClass(const_cast<IReflectObject&>(prototype), name);
					}
				}

				PodInsert(type.pod, (const uint8_t*)"[]", 0, 0, 0, 0, subType.pod);
			}

			PodSetHandler(type.pod, StreamLocateHandler, StreamIterateHandler, &type);
		}

		assert(type.pod->locateHandler == StreamLocateHandler);
		if (type.pod->subList != nullptr || customPersister != nullptr) {
			PodInsert(current->pod, (const uint8_t*)name, (PodSize)((const char*)iter.GetHost() - (const char*)base), 1, 0, 0, type.pod);
		}
	} else {
		// cascade
		bool created = false;
		Type& type = NewType(typeID, typeID->GetName(), postfix, created);
		if (created) {
			if (customPersister != nullptr) {
				type.persister = static_cast<MetaStreamPersist*>(customPersister->Clone());
				PodSetHandler(type.pod, CustomizedLocateHandler, nullptr, type.persister);
			} else {
				OnSubClass(s, name);
			}
		}

		if (type.pod->subList != nullptr || customPersister != nullptr) {
			PodInsert(current->pod, (const uint8_t*)name, (PodSize)((const char*)ptr - (const char*)base), 0, 0, 0, type.pod);
		}
	}
}

void FilterPodImpl::Register(IReflectObject& object) {
	object(*this);
}

void FilterPodImpl::Method(Unique typeID, const char* name, const TProxy<>* p, const Param& retValue, const std::vector<Param>& params, const MetaChainBase* meta) {
	// Not handled now
	assert(false);
}

FilterPodImpl::Type& FilterPodImpl::NewType(Unique id, const String& name, const String& ext, bool& created) {
	// if already registered, skip
	std::map<std::pair<Unique, String>, Type>::iterator p = mapTypes.find(std::make_pair(id, ext));
	if (p == mapTypes.end()) {
		// Create a new pod type
		Pod* pod = PodCreate(root, (const uint8_t*)name.c_str());
		Type& type = mapTypes[std::make_pair(id, ext)];
		type.pod = pod;
		PodSetHandler(pod, DirectLocateHandler, nullptr, &type);
		// add entry
		created = true;
		return type;
	} else {
		created = false;
		return p->second;
	}
}

void FilterPodImpl::Class(IReflectObject& host, Unique id, const char* name, const char* path, const MetaChainBase* meta) {
	bool created = false;
	current = &NewType(id, id->GetName().c_str(), "", created);
	if (created) {
		// write parent fields
		while (meta != nullptr) {
			// Record all metas
			MetaNodeBase* node = const_cast<MetaNodeBase*>(meta->GetNode());
			Unique u = node->GetUnique();
			if (u != UniqueType<Void>::Get()) {
				bool created = false;
				const char* subName = u->GetName().c_str();
				if (mapTypes.find(std::make_pair(u, String(""))) == mapTypes.end()) {
					OnSubClass(*node, subName);
				}

				Type& type = NewType(u, subName, "", created);
				assert(!created); // created already in OnSubClass
				PodInsert(current->pod, (const uint8_t*)subName, 0, 0, 0, 0, type.pod);
			}

			meta = meta->GetNext();
		}
	}
}

IStreamBase* ZFilterPod::CreateFilter(IStreamBase& stream) {
	return new FilterPodImpl(stream);
}

void ZFilterPod::ExportConfig(std::vector<std::pair<String, String> >& config) const {
	// TOOD:
}

void ZFilterPod::ImportConfig(const std::vector<std::pair<String, String> >& config) {
	// TODO:
}