// TPool.h
// PaintDream (paintdream@paintdream.com)
// 2021-2-17
//

#pragma once
#include "TAtomic.h"
#include <stack>

namespace PaintsNow {
	template <class T, class Allocator>
	class TPool {
	public:
		typedef TPool PoolBase;
		TPool(Allocator& alloc, uint32_t count) : allocator(alloc), maxCount(count), currentCount(0), head(nullptr) {}

		~TPool() {
			Clear();
		}

		TPool(rvalue<TPool> rhs) : allocator(((TPool&)rhs).allocator), maxCount(((TPool&)rhs).maxCount), currentCount(((TPool&)rhs).currentCount), head(((TPool&)rhs).head) {
			rhs.head = nullptr;
		}

		T* Acquire() {
			if (head != nullptr) {
				T* p = head;
				head = head->next;
				currentCount--;
				return p;
			} else {
				T* p = allocator.allocate(1);
				allocator.construct(p);
				return p;
			}
		}

		T* AcquireSafe() {
			std::atomic<T*>& h = *reinterpret_cast<std::atomic<T*>*>(&head);
			T* p = (T*)h.exchange(nullptr, std::memory_order_relaxed);

			if (p != nullptr) {
				T* next = p->next;
				p->next = nullptr;

				if (next != nullptr) {
					T* t = (T*)h.exchange(next, std::memory_order_release);
					while (t != nullptr) {
						T* q = t;
						t = t->next;
						q->next = (T*)h.load(std::memory_order_acquire);

						while (!h.compare_exchange_weak(q->next, q, std::memory_order_release)) {}
					}
				}

				std::atomic<uint32_t>& count = *reinterpret_cast<std::atomic<uint32_t>*>(&currentCount);
				count.fetch_sub(1, std::memory_order_release);
				return p;
			} else {
				T* p = allocator.allocate(1);
				allocator.construct(p);
				return p;
			}
		}

		void Release(T* item) {
			assert(item->next == nullptr);
			if (currentCount < maxCount) {
				item->next = head;
				head = item;

				currentCount++;
			} else {
				allocator.destroy(item);
				allocator.deallocate(item, 1);
			}
		}

		void ReleaseSafe(T* item) {
			assert(item->next == nullptr);

			if (currentCount < maxCount) {
				std::atomic<T*>& h = *reinterpret_cast<std::atomic<T*>*>(&head);
				item->next = h.load(std::memory_order_acquire);

				while (!h.compare_exchange_weak(item->next, item, std::memory_order_release)) {}

				std::atomic<uint32_t>& count = *reinterpret_cast<std::atomic<uint32_t>*>(&currentCount);
				count.fetch_add(1, std::memory_order_release);
			} else {
				allocator.destroy(item);
				allocator.deallocate(item, 1);
			}
		}

		void Clear() {
			T* p = head, *q = head;
			while (p != nullptr) {
				p = p->next;
				allocator.destroy(q);
				allocator.deallocate(q, 1);
				q = p;
			}

			head = nullptr;
			currentCount = 0;
		}

	protected:
		TPool& operator = (const TPool& rhs);
		Allocator& allocator;
		uint32_t maxCount;
		uint32_t currentCount;
		T* head;
	};

	template <class T, class H, bool lock = true>
	class TLocalPool : public TPool<T, TLocalPool<T, H, lock> > {
	public:
		TLocalPool(H& h, uint32_t count) : TPool<T, TLocalPool<T, H, lock> >(*this, count), host(h) {}

		T* allocate(size_t n) {
			assert(n == 1);
			if (lock) {
				return host.AcquireSafe();
			} else {
				return host.Acquire();
			}
		}

		void construct(T*) {}
		void destroy(T*) {}

		void deallocate(T* p, size_t n) {
			assert(n == 1);
			if (lock) {
				host.ReleaseSafe(p);
			} else {
				host.Release(p);
			}
		}

	private:
		void ReleaseSafe(T* item);
		T* AcquireSafe();

	protected:
		H& host;
	};
}

