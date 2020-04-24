// Tiny.h
// By PaintDream (paintdream@paintdream.com)
// 2018-1-15
//

#ifndef __TINY_H__
#define __TINY_H__

#include "../Interface/IScript.h"
#include "../Interface/IReflect.h"
#include "../Template/TAtomic.h"

namespace PaintsNow {
	class Tiny : public TReflected<Tiny, IScript::Object> {
	public:
		typedef uint32_t FLAG;
		enum {
			TINY_ACTIVATED = 1U << 0,
			TINY_MODIFIED = 1 << 1,
			TINY_UPDATING = 1 << 2,
			TINY_PINNED = 1U << 3,
			TINY_READONLY = 1U << 4,
			TINY_UNIQUE = 1U << 5,
			TINY_CUSTOM_BEGIN = 1U << 6,
			TINY_CUSTOM_END = 1U << 31,
			TINY_SIZE_BOUND = 1U << 31
		};

		Tiny(FLAG flag = 0);
		TAtomic<FLAG>& Flag();
		FLAG Flag() const;

		virtual TObject<IReflect>& operator () (IReflect& reflect) override;

	protected:
		TAtomic<FLAG> flag;
	};

	class SharedTiny : public TReflected<SharedTiny, Tiny> {
	public:
		// For faster initialization, we do not counting the reference on creating Tiny
		SharedTiny(FLAG f = 0) : BaseClass(f), extReferCount(0) {}
#ifdef _DEBUG
		static void DebugAttach(SharedTiny* host, SharedTiny* tiny);
		static void DebugDetach(SharedTiny* tiny);
#else
		static void DebugAttach(SharedTiny* host, SharedTiny* tiny) {}
		static void DebugDetach(SharedTiny* tiny) {}
#endif
		virtual ~SharedTiny() { SharedTiny::DebugDetach(this); }

		// Make a reference when script object initialized.
		virtual void ScriptInitialize(IScript::Request& request) override {
			ReferenceObject();
		}

		virtual void ReleaseObject() override {
			if (extReferCount.load(std::memory_order_relaxed) == 0) {
				Tiny::ReleaseObject();
			} else {
				--extReferCount;
			}
		}

		virtual void ReferenceObject() {
			++extReferCount;
		}

		uint32_t GetExtReferCount() const {
			return extReferCount.load();
		}

	protected:
		TAtomic<int32_t> extReferCount;
	};

	template <class T>
	class TShared {
	public:
		typedef T Type;
		TShared(T* t = nullptr) {
			tiny = t;
			if (tiny != nullptr) {
				tiny->ReferenceObject();
			}
		}

		static TShared From(T* t) {
			TShared s;
			s.tiny = t;
			return s;
		}

		~TShared() {
			Release();
		}

		TShared(const TShared& s) {
			tiny = s.tiny;
			if (tiny != nullptr)
				tiny->ReferenceObject();
		}

		template <class D>
		operator TShared<D>() {
			return TShared<D>(tiny);
		}

		template <class D>
		static TShared StaticCast(const TShared<D>& src) {
			T* s = static_cast<T*>(src());
			return TShared(s);
		}

		TShared& operator = (const TShared& s) {
			if (s.tiny == tiny) return *this;

			Release();
			tiny = s.tiny;
			if (tiny != nullptr) {
				tiny->ReferenceObject();
			}

			return *this;
		}

		const T& operator * () const {
			return *tiny;
		}

		T& operator * () {
			return *tiny;
		}

		bool operator == (const TShared& m) const {
			return tiny == m.tiny;
		}

		bool operator != (const TShared& m) const {
			return tiny != m.tiny;
		}

		bool operator < (const TShared& m) const {
			return tiny < m.tiny;
		}

		template <class D>
		TShared& Reset(D* t) {
			if (t == tiny) return *this;
			Release();
			tiny = t;

			return *this;
		}

		TShared(rvalue<TShared> s) {
			tiny = s.tiny;
			s.tiny = nullptr;
		}

		TShared& operator = (rvalue<TShared> s) {
			std::swap(tiny, s.tiny);
			return *this;
		}

		void Release() {
			if (tiny != nullptr) {
				tiny->ReleaseObject();
				tiny = nullptr;
			}
		}

		T* operator -> () const {
			assert(tiny);
			return tiny;
		}

		T* operator ()() const {
			return tiny;
		}

		T*& operator () () {
			return tiny;
		}

		operator bool() const {
			return tiny != nullptr;
		}

	private:
		T* tiny;
	};

	template <class T>
	class TSharedCritical {
	public:
		TSharedCritical() : section(0) {}
		TSharedCritical(const TShared<T>& rep) : section(0) {
			*this = rep;
		}

		TSharedCritical<T>& operator = (const TShared<T>& rep) {
			SpinLock(section);
			ptr = rep;
			SpinUnLock(section);
			return *this;
		}

		operator TShared<T> () {
			SpinLock(section);
			TShared<T> res = ptr;
			SpinUnLock(section);
			return std::move(res);
		}

	private:
		TAtomic<int32_t> section;
		TShared<T> ptr;
	};

	template <class T>
	IScript::Request& operator << (IScript::Request& request, const TShared<T>& t) {
		return request << IScript::BaseDelegate(static_cast<IScript::Object*>(t()));
	}
}


#endif // __TINY_H__
