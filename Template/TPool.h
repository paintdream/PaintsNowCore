// TPool.h
// Object Pool
// PaintDream (paintdream@paintdream.com)
//

#pragma once
#include "TAtomic.h"
#include <stack>

namespace PaintsNow {
	// Simple object pool
	template <class D, class T>
	class TPool {
	public:
		typedef TPool PoolBase;
		TPool(size_t count) : maxCount(count) {
			critical.store(0, std::memory_order_release);
		}

		~TPool() {
			Clear();
		}

		T Acquire() {
			if (!freeItems.empty()) {
				T item = freeItems.back();
				freeItems.pop_back();
				return item;
			} else {
				return (static_cast<D*>(this))->New();
			}
		}

		T AcquireSafe() {
			SpinLock(critical);
			if (!freeItems.empty()) {
				T item = freeItems.back();
				freeItems.pop_back();
				return item;
			}
			SpinUnLock(critical);

			return (static_cast<D*>(this))->New();
		}

		void Release(T item) {
			if (freeItems.size() >= maxCount) {
				(static_cast<D*>(this))->Delete(item);
			} else {
				freeItems.emplace_back(item);
			}
		}

		void ReleaseSafe(T item) {
			SpinLock(critical);
			if (freeItems.size() >= maxCount) {
				SpinUnLock(critical);
				(static_cast<D*>(this))->Delete(item);
			} else {
				freeItems.emplace_back(item);
				SpinUnLock(critical);
			}
		}

		void Clear() {
			for (size_t i = 0; i < freeItems.size(); i++) {
				(static_cast<D*>(this))->Delete(freeItems[i]);
			}

			freeItems.clear();
		}

	protected:
		size_t maxCount;
		std::atomic<size_t> critical;
		std::vector<T> freeItems;
	};

	template <class D, class T>
	class TRefPool {
	public:
		typedef TRefPool PoolBase;
		TRefPool(uint32_t count) : maxCount(count), currentCount(0), head(nullptr) {}

		~TRefPool() {
			Clear();
		}

		T* Acquire() {
			if (head != nullptr) {
				T* p = head;
				head = head->next;
				currentCount--;
				return p;
			} else {
				return (static_cast<D*>(this))->New();
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

						while (!h.compare_exchange_weak(q->next, q, std::memory_order_release)) {
							YieldThreadFast();
						}
					}
				}

				std::atomic<uint32_t>& count = *reinterpret_cast<std::atomic<uint32_t>*>(&currentCount);
				count.fetch_sub(1, std::memory_order_release);
				return p;
			} else {
				return (static_cast<D*>(this))->New();
			}
		}

		void Release(T* item) {
			assert(item->next == nullptr);
			if (currentCount < maxCount) {
				item->next = head;
				head = item;

				currentCount++;
			} else {
				(static_cast<D*>(this))->Delete(item);
			}
		}

		void ReleaseSafe(T* item) {
			assert(item->next == nullptr);

			if (currentCount < maxCount) {
				std::atomic<T*>& h = *reinterpret_cast<std::atomic<T*>*>(&head);
				item->next = h.load(std::memory_order_acquire);

				while (!h.compare_exchange_weak(item->next, item, std::memory_order_release)) {
					YieldThreadFast();
				}

				std::atomic<uint32_t>& count = *reinterpret_cast<std::atomic<uint32_t>*>(&currentCount);
				count.fetch_add(1, std::memory_order_release);
			} else {
				(static_cast<D*>(this))->Delete(item);
			}
		}

		void Clear() {
			T* p = head, *q = head;
			while (p != nullptr) {
				p = p->next;
				(static_cast<D*>(this))->Delete(q);
				q = p;
			}

			head = nullptr;
			currentCount = 0;
		}

	protected:
		TRefPool& operator = (const TRefPool& rhs);

		uint32_t maxCount;
		uint32_t currentCount;
		T* head;
	};
}

