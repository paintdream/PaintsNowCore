// Tiny.h
// By PaintDream (paintdream@paintdream.com)
// 2018-1-15
//

#pragma once
#include "../Interface/IScript.h"
#include "../Interface/IReflect.h"
#include "../Template/TAtomic.h"

namespace PaintsNow {
	// A tiny object base class with an atomic flag.
	class Tiny : public TReflected<Tiny, IScript::Object> {
	public:
		typedef uint32_t FLAG;
		enum {
			TINY_ACTIVATED = 1U << 0,		// Tiny is activated
			TINY_MODIFIED = 1 << 1,			// Tiny is modified, need updating
			TINY_UPDATING = 1 << 2,			// Tiny is updating
			TINY_PINNED = 1U << 3,			// Tiny is pinned to another tiny
			TINY_READONLY = 1U << 4,		// Tiny is readonly
			TINY_UNIQUE = 1U << 5,			// Tiny is unique in certain domain
			TINY_CUSTOM_BEGIN = 1U << 6,
			TINY_CUSTOM_END = 1U << 31,
			TINY_SIZE_BOUND = 1U << 31
		};

		Tiny(FLAG flag = 0);
		std::atomic<FLAG>& Flag() { return flag; }
		FLAG Flag() const { return flag.load(std::memory_order_relaxed); }

		TObject<IReflect>& operator () (IReflect& reflect) override;

	protected:
		std::atomic<FLAG> flag;
	};

	// A tiny with builtin reference counting management
	class SharedTiny : public TReflected<SharedTiny, Tiny> {
	public:
		// For faster initialization, we do not count the reference on creating Tiny
		SharedTiny(FLAG f = 0) : BaseClass(f), extReferCount(0) {}
#ifdef _DEBUG
		static void DebugAttach(SharedTiny* host, SharedTiny* tiny);
		static void DebugDetach(SharedTiny* tiny);
#else
		static void DebugAttach(SharedTiny* host, SharedTiny* tiny) {}
		static void DebugDetach(SharedTiny* tiny) {}
#endif
		~SharedTiny() override { SharedTiny::DebugDetach(this); }

		// Make a reference when script object initialized.
		void ScriptInitialize(IScript::Request& request) override {
			ReferenceObject();
		}

		void ReleaseObject() override {
			// every external references released?
			if (extReferCount.load(std::memory_order_relaxed) == 0) {
				Tiny::ReleaseObject();
			} else {
				extReferCount.fetch_sub(1, std::memory_order_relaxed);
			}
		}

		virtual void ReferenceObject() {
			extReferCount.fetch_add(1, std::memory_order_relaxed);
		}

		// Get external refer count, the result is not reliable.
		uint32_t GetExtReferCount() const {
			return extReferCount.load(std::memory_order_relaxed);
		}

	protected:
		std::atomic<int32_t> extReferCount;
	};

	// Shared pointer wrapper for tinies.
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

		// Construct from raw pointer, do not add reference count
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

		// Absorb t as new holding instance
		template <class D>
		TShared& Reset(D* t) {
			if (t == tiny) return *this;

			Release();
			tiny = t;

			return *this;
		}

		TShared(rvalue<TShared> t) {
			TShared& s = t;
			tiny = s.tiny;
			s.tiny = nullptr;
		}

		TShared& operator = (rvalue<TShared> t) {
			TShared& s = t;
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

	// Convert tiny pointer to delegate when it is passed into a script request.
	template <class T>
	IScript::Request& operator << (IScript::Request& request, const TShared<T>& t) {
		return request << IScript::BaseDelegate(static_cast<IScript::Object*>(t()));
	}
}
