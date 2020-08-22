#include "IStreamBase.h"
#include "../Template/TBuffer.h"

using namespace PaintsNow;

const int MAX_INTEGER_SIZE = 64;

inline bool WriteBaseObject(IStreamBase& stream, Unique typeID, Unique refTypeID, void* ptr, size_t size) {
	static Unique strType = UniqueType<String>::Get();
	static Unique byteBufferType = UniqueType<Bytes>::Get();
	bool result = true;
	if (typeID != refTypeID) { // pointer
		IReflectObjectComplex*& object = *reinterpret_cast<IReflectObjectComplex**>(ptr);
		stream << ((object != nullptr) ? object->GetUnique()->GetName() : "");
		if (object != nullptr) {
			return *object >> stream;
		} else {
			return true;
		}
	} else if (typeID == strType) {
		String* str = reinterpret_cast<String*>(ptr);
		if (stream << (uint64_t)str->size()) {
			if (!str->empty()) {
				size_t s = str->size();
				if (!stream.WriteBlock(str->data(), s)) {
					result = false;
				}
			}
		} else {
			result = false;
		}
	} else if (typeID == byteBufferType) {
		Bytes* buffer = reinterpret_cast<Bytes*>(ptr);
		size_t size = buffer->GetSize();
		if (stream << size) {
			if (size != 0) {
				if (!stream.WriteBlock(buffer->GetData(), size)) {
					result = false;
				}
			}
		} else {
			result = false;
		}
	} else {
		if (!stream.WriteBlock(ptr, size)) {
			result = false;
		}
	}

	return result;
}

inline bool ReadBaseObject(IStreamBase& stream, Unique typeID, Unique refTypeID, void* ptr, size_t size) {
	static Unique strType = UniqueType<String>::Get();
	static Unique byteBufferType = UniqueType<Bytes>::Get();
	bool result = true; 
	if (typeID != refTypeID) {
		IReflectObjectComplex*& object = *reinterpret_cast<IReflectObjectComplex**>(ptr);
		String typeName;
		stream >> typeName;
		if (typeName.size() != 0) {
			Unique unique(typeName);
			assert((bool)unique);
			if ((bool)unique && unique->IsCreatable()) {
				object = static_cast<IReflectObjectComplex*>(unique->Create());
				assert(!object->IsBasicObject());
				*object << stream;
				return true;
			} else {
				return false; // object not found
			}
		} else {
			return true;
		}
	} else if (typeID == strType) {
		uint64_t length;
		if (stream >> length) {
			String* str = reinterpret_cast<String*>(ptr);
			if (length < (size_t)-1) {
				str->resize((size_t)length);
				if (length != 0) {
					size_t s = (size_t)length;
					if (!stream.ReadBlock(const_cast<char*>(str->data()), s)) {
						result = false;
					}
				}
			} else {
				result = false;
			}
		} else {
			result = false;
		}
	} else if (typeID == byteBufferType) {
		uint64_t length;
		if (stream >> length) {
			Bytes* buffer = reinterpret_cast<Bytes*>(ptr);
			if (length < (size_t)-1) {
				buffer->Resize(safe_cast<uint32_t>(length));
				if (length != 0) {
					size_t s = safe_cast<uint32_t>(length);
					if (!stream.ReadBlock(buffer->GetData(), s)) {
						result = false;
					}
				}
			} else {
				result = false;
			}
		} else {
			result = false;
		}
	} else {
		if (!stream.ReadBlock(ptr, size)) {
			result = false;
		}
	}

	return result;
}

// this class is not mult-thread safe
template <bool read>
class Reflect : public IReflect {
public:
	Reflect(IStreamBase& s) : IReflect(true, false), stream(s), result(true) {}
	virtual void Property(IReflectObject& s, Unique typeID, Unique refTypeID, const char* name, void* base, void* ptr, const MetaChainBase* meta) {
		if (!result)
			return;

		static Unique metaRuntime = UniqueType<MetaRuntime>::Get();
		const MetaStreamPersist* customPersist = nullptr;

		// check serialization chain
		while (meta != nullptr) {
			// runtime data can not be serialized
			if (meta->GetNode()->GetUnique() == metaRuntime) {
				return;
			} else {
				const MetaStreamPersist* persistHelper = meta->GetNode()->QueryInterface(UniqueType<MetaStreamPersist>());
				if (persistHelper != nullptr) {
					customPersist = persistHelper;
				}
			}

			meta = meta->GetNext();
		}

		size_t size = typeID->GetSize();
		if (s.IsBasicObject()) {
			if (read) {
				if (customPersist != nullptr) {
					if (!customPersist->Read(stream, ptr)) {
						result = false;
					}
				} else {
					if (!ReadBaseObject(stream, typeID, refTypeID, ptr, size)) {
						result = false;
					}
				}
			} else {
				if (customPersist != nullptr) {
					if (!customPersist->Write(stream, ptr)) {
						result = false;
					}
				} else {
					if (!WriteBaseObject(stream, typeID, refTypeID, ptr, size)) {
						result = false;
					}
				}
			}
		} else if (s.IsIterator()) {
			IIterator& it = static_cast<IIterator&>(s);
			uint64_t count;
			if (read) {
				if (stream >> count) {
					it.Initialize((size_t)count);
				} else {
					result = false;
					return;
				}
			} else {
				count = (uint64_t)it.GetTotalCount();
				if (!(stream << count)) {
					result = false;
					return;
				}
			}

			Unique type = it.GetPrototypeUnique();
			bool isPointer = typeID != refTypeID;

			if (it.GetPrototype().IsBasicObject()) {
				while (it.Next()) {
					void* ptr = it.Get();
					size_t length = type->GetSize();
					if (customPersist != nullptr) {
						if (read) {
							if (!customPersist->Read(stream, ptr)) {
								result = false;
							}
						} else {
							if (!customPersist->Write(stream, ptr)) {
								result = false;
							}
						}
					} else {
						if (!isPointer) {
							length *= (size_t)count;
							if (read) {
								if (!stream.ReadBlock(ptr, length)) {
									result = false;
									return;
								}
							} else {
								if (!stream.WriteBlock(ptr, length)) {
									result = false;
									return;
								}
							}

							break; // fast fast fast
						} else {
							// the same as object(*this); but more faster.
							if (read) {
								if (!ReadBaseObject(stream, type, refTypeID, ptr, length)) {
									result = false;
								}
							} else {
								if (!WriteBaseObject(stream, type, refTypeID, ptr, length)) {
									result = false;
								}
							}
						}
					}
				}
			} else {
				while (it.Next()) {
					if (customPersist != nullptr) {
						void* ptr = it.Get();
						if (read) {
							if (!customPersist->Read(stream, ptr)) {
								result = false;
							}
						} else {
							if (!customPersist->Write(stream, ptr)) {
								result = false;
							}
						}
					} else {
						(*reinterpret_cast<IReflectObject*>(it.Get()))(*this);
					}
				}
			}
		} else {
			s(*this); // continue to reflect sub fields
		}
	}
	virtual void Method(Unique typeID, const char* name, const TProxy<>* p, const IReflect::Param& retValue, const std::vector<IReflect::Param>& params, const MetaChainBase* meta) {}
	
	bool GetResult() const { return result; }

private:
	IStreamBase& stream;
	bool result;
};

IStreamBase::IStreamBase() : environment(nullptr) {}
IStreamBase::~IStreamBase() {}

IStreamBase& IStreamBase::GetBaseStream() {
	return *this;
}

bool IStreamBase::WriteForward(const IReflectObject& a, Unique type, Unique refTypeID, void* ptr, size_t length) {
	if (a.IsBasicObject()) {
		return WriteBaseObject(*this, type, refTypeID, ptr, length);
	} else {
		return const_cast<IReflectObject&>(a) >> *this;
	}
}

bool IStreamBase::ReadForward(const IReflectObject& a, Unique type, Unique refTypeID, void* ptr, size_t length) {
	if (a.IsBasicObject()) {
		return ReadBaseObject(*this, type, refTypeID, ptr, length);
	} else {
		return const_cast<IReflectObject&>(a) << *this;
	}
}

bool IStreamBase::Write(IReflectObject& a, Unique type, void* ptr, size_t length) {
	assert(!a.IsBasicObject());
	Reflect<false> reflect(*this);
	reflect.Property(a, type, type, "(nullptr)", ptr, ptr, nullptr);
	return reflect.GetResult();
}

bool IStreamBase::Read(IReflectObject& a, Unique type, void* ptr, size_t length) {
	assert(!a.IsBasicObject());
	Reflect<true> reflect(*this);
	reflect.Property(a, type, type, "(nullptr)", ptr, ptr, nullptr);
	return reflect.GetResult();
}

void IStreamBase::SetEnvironment(IReflectObject& object) {
	environment = &object;
}

IReflectObject& IStreamBase::GetEnvironment() {
	static IReflectObject nullObject;
	return environment == nullptr ? nullObject : *environment;
}

StreamBaseMeasure::StreamBaseMeasure(IStreamBase& base) : stream(base), transition(0) {}

void StreamBaseMeasure::Flush() {
	stream.Flush();
}

bool StreamBaseMeasure::Read(void* p, size_t& len) {
	bool result = stream.Read(p, len);
	transition += len;
	return result;
}

bool StreamBaseMeasure::Write(const void* p, size_t& len) {
	bool result = stream.Write(p, len);
	transition += len;
	return result;
}

bool StreamBaseMeasure::Transfer(IStreamBase& s, size_t& len) {
	bool result = stream.Transfer(s, len);
	transition += len;
	return result;
}

bool StreamBaseMeasure::WriteDummy(size_t& len) {
	bool result = stream.WriteDummy(len);
	transition += len;
	return result;
}

bool StreamBaseMeasure::Seek(SEEK_OPTION option, int64_t offset) {
	bool result = stream.Seek(option, offset);
	assert(option == SEEK_OPTION::CUR);
	transition += offset;
	return result;
}
