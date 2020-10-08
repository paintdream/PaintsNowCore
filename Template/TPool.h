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
}

