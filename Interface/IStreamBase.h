#ifndef __ISTREAMBASE_H__
#define __ISTREAMBASE_H__

#include "IReflect.h"
#include "../PaintsNow.h"
#include <map>
#include <list>
#include <stack>

namespace PaintsNow {
	class IStreamBase : public TReflected<IStreamBase, IReflectObjectComplex> {
	protected:
		virtual ~IStreamBase();
	public:
		IStreamBase();
		virtual void Flush() = 0;
		virtual bool Read(void* p, size_t& len) = 0;
		virtual bool Write(const void* p, size_t& len) = 0;
		virtual bool Transfer(IStreamBase& stream, size_t& len) = 0;
		virtual bool WriteDummy(size_t& len) = 0;
		enum SEEK_OPTION { BEGIN, CUR, END };
		virtual bool Seek(SEEK_OPTION option, long offset) = 0;
		void SetEnvironment(IReflectObject& object);
		IReflectObject& GetEnvironment();

		bool ReadBlock(void* p, size_t& len) {
			size_t org = len;
			if (Read(p, len)) {
				if (len == org) { // done
					return true;
				} else {
					Seek(IStreamBase::CUR, -(long)len);
					return false;
				}
			} else {
				return false;
			}
		}

		bool WriteBlock(const void* p, size_t& len) {
			size_t org = len;
			if (Write(p, len)) {
				if (len == org) {
					return true;
				} else {
					Seek(IStreamBase::CUR, -(long)len);
					return false;
				}
			} else {
				return false;
			}
		}

		class StreamState {
		public:
			StreamState(IStreamBase& s, bool su) : stream(s), success(su) {}
			template <class T>
			inline StreamState operator << (const T& t) {
				assert(success);
				if (success) {
					return stream << t;
				} else {
					return *this;
				}
			}

			template <class T>
			inline StreamState operator >> (const T& t) {
				assert(success);
				if (success) {
					return stream >> t;
				} else {
					return *this;
				}
			}

			inline operator bool() const {
				return success;
			}

			inline bool operator ! () const {
				return !success;
			}

		private:
			IStreamBase& stream;
			bool success;
		};

		template <class T>
		inline StreamState operator << (const T& t) {
			static Unique u = UniqueType<T>::Get();
#if defined(_MSC_VER) && _MSC_VER <= 1200
			static Unique ur = UniqueType<std::remove_pointer<T>::type>::Get();
#else
			static Unique ur = UniqueType<typename std::remove_pointer<T>::type>::Get();
#endif
			bool result = WriteForward(t, u, ur, (void*)&t, sizeof(T));
			return StreamState(*this, result);
		}

		template <class T>
		inline StreamState operator >> (T& t) {
			static Unique u = UniqueType<T>::Get();
#if defined(_MSC_VER) && _MSC_VER <= 1200
			static Unique ur = UniqueType<std::remove_pointer<T>::type>::Get();
#else
			static Unique ur = UniqueType<typename std::remove_pointer<T>::type>::Get();
#endif
			bool result = ReadForward(t, u, ur, (void*)&t, sizeof(T));
			return StreamState(*this, result);
		}

		virtual bool Write(IReflectObject& a, Unique type, void* ptr, size_t length);
		virtual bool Read(IReflectObject& a, Unique type, void* ptr, size_t length);

		template <class T>
		T Parse(UniqueType<T>) {
			T object;
			*this >> object;
			return object;
		}

	protected:
		bool WriteForward(const IReflectObject& a, Unique type, Unique refTypeID, void* ptr, size_t length);
		bool ReadForward(const IReflectObject& a, Unique type, Unique refTypeID, void* ptr, size_t length);

		IReflectObject* environment;
	};

	class StreamBaseMeasure : public IStreamBase {
	public:
		StreamBaseMeasure(IStreamBase& base);
		virtual void Flush() override;
		virtual bool Read(void* p, size_t& len) override;
		virtual bool Write(const void* p, size_t& len) override;
		virtual bool Transfer(IStreamBase& stream, size_t& len) override;
		virtual bool WriteDummy(size_t& len) override;
		virtual bool Seek(SEEK_OPTION option, long offset) override;

		IStreamBase& stream;
		int64_t transition;
	};

	class MetaStreamPersist : public TReflected<MetaStreamPersist, MetaNodeBase> {
	public:
		virtual bool Read(IStreamBase& streamBase, void* ptr) const = 0;
		virtual bool Write(IStreamBase& streamBase, const void* ptr) const = 0;
		virtual const String& GetUniqueName() const = 0;
	};
}


#endif // __ISTREAMBASE_H__
