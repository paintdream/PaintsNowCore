// TAllocator.h
// PaintDream (paintdream@paintdream.com)
// 2019-10-9
//

#pragma once
#include "../Interface/IMemory.h"
#include "TAtomic.h"
#include "TAlgorithm.h"
#include "../System/Tiny.h"
#include <vector>
#include <utility>

namespace PaintsNow {
	// Global allocator that allocates memory blocks to local allocators.
	template <size_t N, size_t K>
	class TRootAllocator {
	public:
		void* Allocate() {
			static_assert(K < sizeof(size_t) * 8, "K is too large for root allocators.");

			// do fast operations in critical section
			SpinLock(critical);
			for (size_t i = 0; i < blocks.size(); i++) {
				Block& block = blocks[i];
				if (block.bitmap != ((size_t)1 << K) - 1) {
					size_t bit = block.bitmap + 1;
					bit = bit & (~bit + 1);
					size_t index = Math::Log2(bit);
					block.bitmap |= bit;

					SpinUnLock(critical);
					return block.address + index * N;
				}
			}

			SpinUnLock(critical);

			// real allocation, release the critical.
			Block block;
			block.address = reinterpret_cast<uint8_t*>(IMemory::AllocAligned(N * K, N));
			block.bitmap = 1;

			// write result back
			SpinLock(critical);
			blocks.emplace_back(block);
			SpinUnLock(critical);

			return block.address;
		}

		void Deallocate(void* p) {
			void* t = nullptr;

			SpinLock(critical);

			// loop to find required one.
			for (size_t i = 0; i < blocks.size(); i++) {
				Block& block = blocks[i];
				if (p >= block.address && p < block.address + N * K) {
					size_t index = (reinterpret_cast<uint8_t*>(p) - block.address) / N;
					assert(block.bitmap & ((size_t)1 << index));
					block.bitmap &= ~((size_t)1 << index);

					if (block.bitmap == 0) {
						t = block.address;
						blocks.erase(blocks.begin() + i);
					}

					break;
				}
			}

			SpinUnLock(critical);

			if (t != nullptr) {
				// do free
				IMemory::FreeAligned(t);
			}
		}

		// We are not dll-friendly, as always.
		static TRootAllocator& Get() {
			static TRootAllocator allocator;
			return allocator;
		}

	protected:
		struct Block {
			uint8_t* address;
			size_t bitmap;
		};

	protected:
		std::atomic<size_t> critical;
		std::vector<Block> blocks;
	};

	// Local allocator, allocate memory with specified alignment requirements.
	// K = element size, M = block size, R = max recycled block count, 0 for not limited
	template <size_t K, size_t M = 8192, size_t R = 8, size_t S = sizeof(size_t) * 8 - 1>
	class TAllocator : public TReflected<TAllocator<K, M, R>, SharedTiny> {
	public:
		typedef TReflected<TAllocator<K, M, R>, SharedTiny> BaseClass;
		enum {
			SIZE = M,
			N = M / K,
			BITS = 8 * sizeof(size_t),
			BITMAPSIZE = (N + BITS - 1) / BITS,
			MASK = BITS - 1
		};

		struct ControlBlock {
			TAllocator* allocator;
			std::atomic<size_t> refCount;
			std::atomic<size_t> recycled;
			std::atomic<size_t> bitmap[BITMAPSIZE];
		};

		enum {
			OFFSET = (sizeof(ControlBlock) + K - 1) / K
		};

	public:
		TAllocator() {
			static_assert(N / 2 * K > sizeof(ControlBlock), "N is too small");
			critical.store(0, std::memory_order_relaxed);
			recycleCount.store(0, std::memory_order_relaxed);
			controlBlock.store(nullptr, std::memory_order_release);
		}

		~TAllocator() override {
			ControlBlock* p = (ControlBlock*)controlBlock.load(std::memory_order_acquire);

			// deallocate all caches
			TRootAllocator<M, S>& allocator = TRootAllocator<M, S>::Get();
			if (p != nullptr) {
				allocator.Deallocate(p);
			}

			for (size_t i = 0; i < recycled.size(); i++) {
				if (recycled[i] != p) {
					allocator.Deallocate(recycled[i]);
				}
			}
		}

		inline void* Allocate() {
			while (true) {
				ControlBlock* p = (ControlBlock*)controlBlock.exchange(nullptr, std::memory_order_acquire);

				if (p == nullptr) {
					// need a new block
					if (critical.load(std::memory_order_acquire) == 2u) {
						SpinLock(critical);
						if (!recycled.empty()) {
							p = recycled.back();
							recycled.pop_back();

							p->recycled.store(0, std::memory_order_relaxed);
						}
						SpinUnLock(critical, (size_t)(recycled.empty() ? 0u : 2u));

						recycleCount.fetch_sub(1, std::memory_order_relaxed);
						// no elements?
						if (p != nullptr) {
							assert(p->refCount.load(std::memory_order_acquire) >= 1);
						}
					}

					if (p == nullptr) {
						p = reinterpret_cast<ControlBlock*>(TRootAllocator<M, S>::Get().Allocate());
						memset(p, 0, sizeof(ControlBlock));
						p->allocator = this;
						p->refCount.store(1, std::memory_order_relaxed); // newly allocated one, just set it to 1
					}
				}

				// search for an empty slot
				for (size_t k = 0; k < BITMAPSIZE; k++) {
					std::atomic<size_t>& s = p->bitmap[k];
					size_t mask = s.load(std::memory_order_acquire);
					if (mask != ~(size_t)0) {
						size_t bit = Math::Alignment(mask + 1);
						if (!(s.fetch_or(bit, std::memory_order_relaxed) & bit)) {
							// get index of bitmap
							size_t index = Math::Log2(bit) + OFFSET + k * 8 * sizeof(size_t);
							if (index < N) {
								p->refCount.fetch_add(1, std::memory_order_relaxed);

								BaseClass::ReferenceObject();
								// add to recycle system if needed
								Recycle(p);

								return reinterpret_cast<char*>(p) + index * K;
							}
						}
					}
				}

				// full?
				TryFree(p);
			}

			assert(false);
			return nullptr; // never reach here
		}

		static inline void Deallocate(void* ptr) {
			size_t t = reinterpret_cast<size_t>(ptr);
			ControlBlock* p = reinterpret_cast<ControlBlock*>(t & ~(SIZE - 1));
			size_t id = (t - (size_t)p) / K - OFFSET;
			p->allocator->Deallocate(p, id);
		}

	protected:
		inline void TryFree(ControlBlock* p) {
			assert(p->refCount.load(std::memory_order_acquire) != 0);
			if (p->refCount.fetch_sub(1, std::memory_order_release) == 1) {
				assert(controlBlock.load(std::memory_order_acquire) != p);
				TRootAllocator<M, S>::Get().Deallocate(p);
			}
		}

		inline void Recycle(ControlBlock* p) {
			assert(p->refCount.load(std::memory_order_acquire) != 0);

			ControlBlock* expected = nullptr;
			if (!controlBlock.compare_exchange_strong(expected, p, std::memory_order_relaxed)) {
				// search for recycled
				if (recycleCount.load(std::memory_order_acquire) < R && p->recycled.load(std::memory_order_acquire) == 0) {
					SpinLock(critical);
					if (p->recycled.exchange(1, std::memory_order_acquire) == 0) {
						std::binary_insert(recycled, p);
						recycleCount.fetch_add(1, std::memory_order_relaxed);
						SpinUnLock(critical, (size_t)2u);

						// recycled succeed
						return;
					}
					SpinUnLock(critical, (size_t)(recycled.empty() ? 0u : 2u));
				}

				TryFree(p);
			}
		}

		inline void Deallocate(ControlBlock* p, size_t id) {
			assert(p->allocator != nullptr);
			p->bitmap[id / BITS].fetch_and(~((size_t)1 << (id & MASK)));

			Recycle(p);
			BaseClass::ReleaseObject();
		}

	protected:
		std::atomic<ControlBlock*> controlBlock;
		std::atomic<size_t> critical;
		std::atomic<size_t> recycleCount;
		std::vector<ControlBlock*> recycled;
	};

	// Allocate for objects
	template <class T, size_t M = 8192, size_t Align = 64, size_t R = 8>
	class TObjectAllocator : protected TAllocator<(sizeof(T) + Align - 1) & ~(Align - 1), M, R> {
	public:
		typedef TAllocator<(sizeof(T) + Align - 1) & ~(Align - 1), M, R> Base;
		static inline void Delete(T* object) {
			object->~T();
			Base::Deallocate(object);
		}

		void ReferenceObject() override {
			Base::ReferenceObject();
		}

		void ReleaseObject() override {
			Base::ReleaseObject();
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		inline T* New() {
			void* ptr = Allocate();
			return new (ptr) T();
		}

		template <class A>
		inline T* New(A a) {
			void* ptr = Allocate();
			return new (ptr) T(a);
		}

		template <class A, class B>
		inline T* New(A a, B b) {
			void* ptr = Allocate();
			return new (ptr) T(a, b);
		}

		template <class A, class B, class C>
		inline T* New(A a, B b, C c) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c);
		}

		template <class A, class B, class C, class D>
		inline T* New(A a, B b, C c, D d) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d);
		}

		template <class A, class B, class C, class D, class E>
		inline T* New(A a, B b, C c, D d, E e) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e);
		}

		template <class A, class B, class C, class D, class E, class F>
		inline T* New(A a, B b, C c, D d, E e, F f) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f);
		}

		template <class A, class B, class C, class D, class E, class F, class G>
		inline T* New(A a, B b, C c, D d, E e, F f, G g) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);
		}

		template <class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P>
		inline T* New(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) {
			void* ptr = Allocate();
			return new (ptr) T(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p);
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
		typedef TAllocatedTiny BaseClass;
#if defined(_MSC_VER) && _MSC_VER <= 1200
		TAllocatedTiny() {}
		template <class A>
		TAllocatedTiny(A& a) : TReflected<T, B>(a) {}
		template <class A, class B>
		TAllocatedTiny(A& a, B& b) : TReflected<T, B>(a, b) {}
		template <class A, class B, class C>
		TAllocatedTiny(A& a, B& b, C& c) : TReflected<T, B>(a, b, c) {}
		template <class A, class B, class C, class D>
		TAllocatedTiny(A& a, B& b, C& c, D& d) : TReflected<T, B>(a, b, c, d) {}
		template <class A, class B, class C, class D, class E>
		TAllocatedTiny(A& a, B& b, C& c, D& d, E& e) : TReflected<T, B>(a, b, c, d, e) {}
		template <class A, class B, class C, class D, class E, class F>
		TAllocatedTiny(A& a, B& b, C& c, D& d, E& e, F& f) : TReflected<T, B>(a, b, c, d, e, f) {}
		template <class A, class B, class C, class D, class E, class F, class G>
		TAllocatedTiny(A& a, B& b, C& c, D& d, E& e, F& f, G& g) : TReflected<T, B>(a, b, c, d, e, f, g) {}
		template <class A, class B, class C, class D, class E, class F, class G, class H>
		TAllocatedTiny(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h) : TReflected<T, B>(a, b, c, d, e, f, g, h) {}
#else
		template <typename... Args>
		TAllocatedTiny(Args&&... args) : TReflected<T, B>(std::forward<Args>(args)...) {}
#endif

		void FinalDestroy() override {
			Allocator::Delete(static_cast<T*>(this));
		}
	};
}
