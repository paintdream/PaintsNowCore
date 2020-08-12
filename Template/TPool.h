// TPool.h
// Object Pool
// PaintDream (paintdream@paintdream.com)
//

#ifndef __TPOOL_H__
#define __TPOOL_H__

#include "TAtomic.h"
#include <stack>

namespace PaintsNow {
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
				T item = freeItems.top();
				freeItems.pop();
				return item;
			} else {
				return (static_cast<D*>(this))->New();
			}
		}

		T AcquireSafe() {
			SpinLock(critical);
			if (!freeItems.empty()) {
				T item = freeItems.top();
				freeItems.pop();
				return item;
			}
			SpinUnLock(critical);

			return (static_cast<D*>(this))->New();
		}

		void Release(T item) {
			if (freeItems.size() >= maxCount) {
				(static_cast<D*>(this))->Delete(item);
			} else {
				freeItems.push(item);
			}
		}

		void ReleaseSafe(T item) {
			SpinLock(critical);
			if (freeItems.size() >= maxCount) {
				SpinUnLock(critical);
				(static_cast<D*>(this))->Delete(item);
			} else {
				freeItems.push(item);
				SpinUnLock(critical);
			}
		}

		void Clear() {
			while (!freeItems.empty()) {
				(static_cast<D*>(this))->Delete(freeItems.top());
				freeItems.pop();
			}
		}

	protected:
		size_t maxCount;
		std::atomic<uint32_t> critical;
		std::stack<T> freeItems;
	};
}

#endif // __TPOOL_H__
