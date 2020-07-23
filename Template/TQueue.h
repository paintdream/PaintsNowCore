// TQueue.h
// One to one queue
// PaintDream (paintdream@paintdream.com)
//

#ifndef __TQUEUE_H__
#define __TQUEUE_H__

#include <vector>
#include <cassert>
#include "TAtomic.h"

namespace PaintsNow {
	// Single-read-single-write
	template <class T, size_t K = 8>
	class TQueue {
	public:
		TQueue() : pushIndex(0), popIndex(0) {}

		enum {
			N = 1 << K,
			Mask = N - 1
		};

		struct Iterator {
			Iterator(uint32_t i = 0) : index(i) {}
			Iterator& operator ++() {
				index = (index + 1) & Mask;
				return *this;
			}

			bool operator == (const Iterator& rhs) const {
				return index == rhs.index;
			}

			bool operator != (const Iterator& rhs) const {
				return index != rhs.index;
			}

			uint32_t index;
		};

#if defined(_MSC_VER) && _MSC_VER <= 1200
		inline bool Push(const T& t) {
			uint32_t nextIndex = (pushIndex + 1) & Mask;
			if (nextIndex == popIndex) {
				return false; // full
			}

			ringBuffer[pushIndex] = t;
			std::atomic_thread_fence(std::memory_order_release);
			pushIndex = nextIndex;
			return true;
		}

		template <class D>
		inline bool Push(rvalue<D> t) {
			uint32_t nextIndex = (pushIndex + 1) & Mask;
			if (nextIndex == popIndex) {
				return false; // full
			}

			ringBuffer[pushIndex] = t;
			std::atomic_thread_fence(std::memory_order_release);
			pushIndex = nextIndex;
			return true;
		}
#else
		template <class D>
		inline bool Push(D&& t) {
			uint32_t nextIndex = (pushIndex + 1) & Mask;
			if (nextIndex == popIndex) {
				return false; // full
			}

			ringBuffer[pushIndex] = std::forward<D>(t);
			std::atomic_thread_fence(std::memory_order_release);
			pushIndex = nextIndex;
			return true;
		}
#endif

		inline T& Top() {
			std::atomic_thread_fence(std::memory_order_acquire);
			assert(!Empty());
			return ringBuffer[popIndex];
		}

		inline const T& Top() const {
			std::atomic_thread_fence(std::memory_order_acquire);
			assert(!Empty());
			return ringBuffer[popIndex];
		}

		// for prefetch
		inline const T& Predict() const {
			return ringBuffer[(popIndex + 1) & Mask];
		}

		inline void Pop() {
			popIndex = (popIndex + 1) & Mask;
		}

		template <class F>
#if defined(_MSC_VER) && _MSC_VER <= 1200
		inline bool Iterate(F& f) {
#else
		inline bool Iterate(F&& f) {
#endif
			for (uint32_t i = popIndex; i != pushIndex; i = (i + 1) & Mask) {
				T& predict = ringBuffer[(i + 1) & Mask];

#if defined(_MSC_VER) && _MSC_VER <= 1200
				if (!f(ringBuffer[i], predict)) {
#else
				if (!(std::forward<F>(f))(ringBuffer[i], predict)) {
#endif
					return false;
				}
			}

			return true;
		}

		inline bool Empty() const {
			return popIndex == pushIndex;
		}
		
		inline uint32_t Count() const {
			return (popIndex + Mask - pushIndex) % Mask;
		}

		inline Iterator Begin() const {
			return popIndex;
		}

		inline Iterator End() const {
			return pushIndex;
		}

		inline T& operator [] (Iterator it) {
			return ringBuffer[it.index];
		}

		inline const T& operator [] (Iterator it) const {
			return ringBuffer[it.index];
		}

	protected:
		uint32_t pushIndex;
		uint32_t popIndex;
		T ringBuffer[N];
	};

	// Still single-read-single-write
	template <class T, size_t K = 8>
	class TQueueList {
	public:
		typedef TQueue<T, K> SubQueue;
		class Node : public SubQueue {
		public:
			Node() : next(nullptr) {}
			Node* next;
		};

	protected:
		Node* pushHead;
		Node* popHead; // popHead is always prior to pushHead

	public:
		TQueueList() {
			pushHead = popHead = new Node();
		}

		TQueueList(const TQueueList& rhs) {
			assert(rhs.Empty());
		}

		TQueueList(rvalue<TQueueList> rhs) {
			pushHead = rhs.pushHead;
			popHead = rhs.popHead;

			rhs.pushHead = nullptr;
			rhs.popHead = nullptr;
		}

		TQueueList& operator = (rvalue<TQueueList> rhs) {
			std::swap(pushHead, rhs.pushHead);
			std::swap(popHead, rhs.popHead);

			return *this;
		}

		// not a thread safe destructor.
		~TQueueList() {
			if (popHead != nullptr) {
				// free momery
				Node* q = popHead;
				while (q != nullptr) {
					Node* p = q;
					q = q->next;
					delete p;
				}
			}
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		template <class D>
		inline void Push(D& t) {
			if (!pushHead->Push(t)) { // full
				Node* p = new Node();
				bool success = p->Push(t); // Must success
				assert(success);

				pushHead->next = p;
				std::atomic_thread_fence(std::memory_order_release);
				pushHead = p;
			}
		}
#else
		template <class D>
		inline void Push(D&& t) {
			if (!pushHead->Push(std::forward<D>(t))) { // full
				Node* p = new Node();
				bool success = p->Push(std::forward<D>(t)); // Must success
				assert(success);

				pushHead->next = p;
				std::atomic_thread_fence(std::memory_order_release);
				pushHead = p;
			}
		}
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1200
		template <class D>
		inline Node* QuickPush(D& t, Node* storage) {
			if (!pushHead->Push(t)) { // full
				Node* p = storage;
				bool success = p->Push(t); // Must success
				assert(success);

				pushHead->next = p;
				std::atomic_thread_fence(std::memory_order_release);
				pushHead = p;
				return nullptr;
			} else {
				return storage;
			}
		}
#else
		template <class D>
		inline Node* QuickPush(D&& t, Node* storage) {
			if (!pushHead->Push(std::forward<D>(t))) { // full
				Node* p = storage;
				bool success = p->Push(std::forward<D>(t)); // Must success
				assert(success);

				pushHead->next = p;
				std::atomic_thread_fence(std::memory_order_release);
				pushHead = p;
				return nullptr;
			} else {
				return storage;
			}
		}
#endif

		inline T& Top() {
			return popHead->Top();
		}

		inline const T& Top() const {
			return popHead->Top();
		}

		inline const T& Predict() const {
			return popHead->Predict();
		}

		inline Node* QuickPop() {
			popHead->Pop();
			if (popHead->Empty() && popHead != pushHead) {
				Node* p = popHead;
				popHead = popHead->next;
				return p;
			} else {
				return nullptr;
			}
		}

		inline void Pop() {
			popHead->Pop();
			if (popHead->Empty() && popHead != pushHead) {
				Node* p = popHead;
				popHead = popHead->next;
				delete p;
			}
		}

		inline bool Empty() const {
			return popHead->Empty();
		}

		struct Iterator {
			Iterator(Node* n, typename SubQueue::Iterator t) : p(n), it(t) {}
			Iterator& operator ++ () {
				if (++it == p->End()) {
					Node* t = p->next;
					if (t != nullptr) {
						p = t;
						it = p->Begin();
					}
				}

				return *this;
			}

			bool operator == (const Iterator& t) const {
				return p == t.p && it == t.it;
			}

			bool operator != (const Iterator& t) const {
				return p != t.p || it != t.it;
			}

			Node* p;
			typename SubQueue::Iterator it;
		};

		inline Iterator Begin() const {
			return Iterator(popHead, popHead->Begin());
		}

		inline Iterator End() const {
			return Iterator(pushHead, pushHead->End());
		}

		inline uint32_t Count() const {
			uint32_t counter = 0;
			for (Node* p = popHead; p != nullptr; p = p->next) {
				counter += p->Count();
			}

			return counter;
		}

		inline const T& operator [] (Iterator it) const {
			return (*it->p)[it];
		}

		inline T& operator [] (Iterator it) {
			return (*it->p)[it];
		}

		template <class F>
#if defined(_MSC_VER) && _MSC_VER <= 1200
		inline bool Iterate(F& f) {
#else
		inline bool Iterate(F&& f) {
#endif
			for (Node* p = popHead; p != nullptr; p = p->next) {
#if defined(_MSC_VER) && _MSC_VER <= 1200
				if (!p->Iterate(f)) {
#else
				if (!p->Iterate(std::forward<F>(f))) {
#endif
					return false;
				}
			}

			return true;
		}
	};
}

#endif // __TQUEUE_H__