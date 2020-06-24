// TAllocator.h
// PaintDream (paintdream@paintdream.com)
// 2019-10-9
//

#ifndef __TALLOCATOR_H__
#define __TALLOCATOR_H__

#include "../Interface/IMemory.h"
#include "TAtomic.h"
#include "TAlgorithm.h"
#include "../System/Tiny.h"
#include <vector>
#include <utility>

namespace PaintsNow {
	template <size_t K, size_t M = 8192>
	class TAllocator : public TReflected<TAllocator<K, M>, SharedTiny> {
	public:
		typedef TReflected<TAllocator<K, M>, SharedTiny> BaseClass;
		enum {
			SIZE = M,
			N = M / K,
			BITS = 8 * sizeof(size_t),
			BITMAPSIZE = (N + BITS - 1) / BITS,
			MASK = BITS - 1
		};

		struct ControlBlock {
			TAllocator* allocator;
			TAtomic<uint32_t> referenced;
			TAtomic<uint32_t> allocCount;
			TAtomic<size_t> bitmap[BITMAPSIZE];
		};

		enum {
			OFFSET = (sizeof(ControlBlock) + K - 1) / K
		};

	public:
		TAllocator() {
			static_assert(N / 2 * K > sizeof(ControlBlock), "N is too small");
			controlBlock.store(nullptr, std::memory_order_relaxed);
			critical.store(0, std::memory_order_relaxed);
		}

		~TAllocator() {
			ControlBlock* p = (ControlBlock*)controlBlock.load(std::memory_order_acquire);
			if (p != nullptr) {
#ifdef _DEBUG
				SpinLock(critical);
				marked.erase(p);
				SpinUnLock(critical);
#endif
				IMemory::FreeAligned(p);
			}

			// SpinLock(critical);
			for (size_t i = 0; i < recycledBlocks.size(); i++) {
				assert(p->allocCount.load(std::memory_order_acquire) == 0);
#ifdef _DEBUG
				SpinLock(critical);
				marked.erase(p);
				SpinUnLock(critical);
#endif
				IMemory::FreeAligned(p);
			}
			// SpinUnLock(critical);

#ifdef _DEBUG
			assert(marked.empty()); // guard for memory leaks
#endif
		}

		inline void* Allocate() {
			while (true) {
				ControlBlock* p = (ControlBlock*)controlBlock.load(std::memory_order_acquire);
				if (p == nullptr) {
					SpinLock(critical);
					if (!recycledBlocks.empty()) {
						p = recycledBlocks.back();
						recycledBlocks.pop_back();
					}
					SpinUnLock(critical);

					if (p == nullptr) {
						p = reinterpret_cast<ControlBlock*>(IMemory::AllocAligned(SIZE, SIZE));
						memset(p, 0, sizeof(ControlBlock));
#ifdef _DEBUG
						SpinLock(critical);
						marked.insert(p);
						SpinUnLock(critical);
#endif
						p->referenced.store(1, std::memory_order_relaxed);
						p->allocator = this;
					}

					ControlBlock* w = (ControlBlock*)controlBlock.exchange(p, std::memory_order_release);
					if (w != nullptr) {
						assert(w->allocator != nullptr);
						assert(w->referenced.load(std::memory_order_acquire) != 0);
						SpinLock(critical);
						recycledBlocks.emplace_back(w);
						SpinUnLock(critical);
					}
				}

				for (size_t k = 0; k < BITMAPSIZE; k++) {
					TAtomic<size_t>& s = p->bitmap[k];
					size_t mask = s.load(std::memory_order_relaxed);
					if (mask != ~(size_t)0) {
						size_t bit = Alignment(mask + 1);
						size_t index = Log2(bit) + OFFSET + k * 8 * sizeof(size_t);
						if (index < N && s.compare_exchange_strong(mask, mask | bit, std::memory_order_release)) {
							if (p->allocCount.fetch_add(1, std::memory_order_relaxed) == 0) {
								BaseClass::ReferenceObject();
							}

							return reinterpret_cast<char*>(p) + index * K;
						}
					}
				}

				// full, remove
				if (controlBlock.compare_exchange_strong(p, nullptr, std::memory_order_relaxed)) {
					p->referenced.store(0, std::memory_order_release);
				}
			}

			return nullptr; // never reach here
		}

		static inline void Deallocate(void* ptr) {
			size_t t = reinterpret_cast<size_t>(ptr);
			ControlBlock* p = reinterpret_cast<ControlBlock*>(t & ~(SIZE - 1));
			size_t id = (t - (size_t)p) / K - OFFSET;
			p->allocator->Deallocate(p, id);
		}

	protected:
		inline bool CleanRecycled(ControlBlock* t) {
			if (t->referenced.load(std::memory_order_relaxed) == 0) {
				return true;
			}

			bool find = false;
			SpinLock(critical);
			for (size_t i = 0; i < recycledBlocks.size(); i++) {
				ControlBlock* p = recycledBlocks[i];
				if (p == t) {
					recycledBlocks.erase(recycledBlocks.begin() + i);
					find = true;
					break;
				}
			}
			SpinUnLock(critical);

			return find;
		}

		inline void Deallocate(ControlBlock* p, size_t id) {
			assert(p->allocator != nullptr);

			bool release = p->allocCount.fetch_sub(1, std::memory_order_release) == 1;
			if (release) {
				if (CleanRecycled(p)) {
#ifdef _DEBUG
					SpinLock(critical);
					marked.erase(p);
					SpinUnLock(critical);
#endif
					IMemory::FreeAligned(p);
					BaseClass::ReleaseObject();
					return;
				}

				release = true;
			}

			p->bitmap[id / BITS].fetch_and(~((size_t)1 << (id & MASK)));
			if (p->referenced.exchange(1, std::memory_order_relaxed) == 0) {
				SpinLock(critical);
				recycledBlocks.emplace_back(p);
				SpinUnLock(critical);
			}

			if (release) {
				BaseClass::ReleaseObject();
			}
		}

	protected:
		TAtomic<ControlBlock*> controlBlock;
		TAtomic<uint32_t> critical;
		std::vector<ControlBlock*> recycledBlocks;
#ifdef _DEBUG
		std::set<ControlBlock*> marked;
#endif
	};

	template <class T, size_t M = 8192, size_t Align = 64>
	class TObjectAllocator : protected TAllocator<(sizeof(T) + Align - 1) & ~(Align - 1), M> {
	public:
		typedef TAllocator<(sizeof(T) + Align - 1) & ~(Align - 1), M> Base;
		static inline void Delete(T* object) {
			object->~T();
			Base::Deallocate(object);
		}

		virtual void ReferenceObject() override {
			Base::ReferenceObject();
		}

		virtual void ReleaseObject() override {
			Base::ReleaseObject();
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		inline T* New() {
			void* ptr = Allocate();
			return new (ptr) T();
		}

		template <class A>
		inline T* New(A& a) {
			void* ptr = Allocate();
			return new (ptr) T(a);
		}

		template <class A, class B>
		inline T* New(A& a, B& b) {
			void* ptr = Allocate();
			return new (ptr) T(a, b);
		}

		template <class A, class B, class C>
		inline T* New(A& a, B& b, C& c) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c);
		}

		template <class A, class B, class C, class D>
		inline T* New(A& a, B& b, C& c, D& d) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d);
		}

		template <class A, class B, class C, class D, class E>
		inline T* New(A& a, B& b, C& c, D& d, E& e) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e);
		}

		template <class A, class B, class C, class D, class E, class F>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f);
		}

		template <class A, class B, class C, class D, class E, class F, class G>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i, J& j) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i, J& j, K& k) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i, J& j, K& k, L& l) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i, J& j, K& k, L& l, M& m) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
		inline T* New(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h, I& i, J& j, K& k, L& l, M& m, N& n) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}
#else
		template <typename... Args>
		inline T* New(Args&&... args) {
			void* ptr = Base::Allocate();
			return new (ptr) T(std::forward<Args>(args)...);
		}
#endif
	};

	template <class T, class B, size_t M = 8192, size_t Align = 64>
	class TAllocatedTiny : public TReflected<T, B> {
	public:
		typedef TObjectAllocator<T, M, Align> Allocator;

		virtual void FinalDestroy() override {
			Allocator::Delete(static_cast<T*>(this));
		}
	};
}

#endif // __TALLOCATOR_H__