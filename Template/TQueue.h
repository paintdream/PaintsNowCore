// TQueue.h
// One to one queue, kfifo.
// PaintDream (paintdream@paintdream.com)
//

#pragma once
#include <vector>
#include <cassert>
#include "TAtomic.h"
#include "TAlgorithm.h"
#include "TAllocator.h"

namespace PaintsNow {
	// Single-read-single-write
	template <class T, size_t K = 8, size_t C = 16>
	class TQueue {
	public:
		TQueue(rvalue<TQueue> q) {
			TQueue& queue = q;
			pushIndex = queue.pushIndex;
			popIndex = queue.popIndex;
			ringBuffer = queue.ringBuffer;
			next = queue.next;
			queue.ringBuffer = nullptr;
		}

		enum {
			N = 1 << K,
			Mask = N - 1
		};

		typedef TRootAllocator<sizeof(T) * N, C> Allocator;
		TQueue() : pushIndex(0), popIndex(0), next(nullptr) {
			// leave uninitialized
			ringBuffer = new (Allocator::Get().Allocate()) T[N];
		}

		~TQueue() {
			if (ringBuffer != nullptr) {
				for (size_t i = 0; i < N; i++) {
					ringBuffer[i].~T();
				}

				Allocator::Get().Deallocate(ringBuffer);
			}
		}

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
		inline void Pop() {
			std::atomic_thread_fence(std::memory_order_acquire);
			popIndex = (popIndex + 1) & Mask;
		}

		// for prefetch
		inline const T& Predict() const {
			return ringBuffer[(popIndex + 1) & Mask];
		}

		inline T* Allocate(uint32_t count, uint32_t alignment) {
			assert(count >= alignment);
			assert(count < N);
			// Make alignment
			count += (uint32_t)(alignment - Math::Alignment(pushIndex)) & (alignment - 1);
			if (count >= N - 1 - Count()) return nullptr;

			uint32_t retIndex = pushIndex;
			uint32_t nextIndex = pushIndex + count;
			if (count != 1 && nextIndex >= N) return nullptr; // non-continous!

			nextIndex = nextIndex & Mask;
			pushIndex = nextIndex;

			return ringBuffer + retIndex;
		}

		inline void Deallocate(uint32_t count, uint32_t alignment) {
			assert(count >= alignment);
			assert(count < N);

			// Make alignment
			count += (uint32_t)(alignment - Math::Alignment(popIndex)) & (alignment - 1);
			assert(count <= Count());
			popIndex = (popIndex + count) & Mask;
		}

		inline void Reset() {
			pushIndex = popIndex = 0;
		}

		inline T& Top() {
			assert(!Empty());
			return ringBuffer[popIndex];
		}

		inline const T& Top() const {
			assert(!Empty());
			return ringBuffer[popIndex];
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
			return (pushIndex + N - popIndex) & Mask;
		}

		inline uint32_t GetPackCount(uint32_t alignment) const {
			assert(N >= alignment);
			uint32_t index = pushIndex + (uint32_t)(alignment - Math::Alignment(pushIndex)) & (alignment - 1);

			if (popIndex > pushIndex) {
				return popIndex > index ? popIndex - index : 0;
			} else {
				return N - index;
			}
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

		static uint32_t GetFullPackCount() {
			return N;
		}

	protected:
		uint32_t pushIndex;
		uint32_t popIndex;
		T* ringBuffer;
	public:
		TQueue* next;
	};

	// Still single-read-single-write
	template <class T, size_t K = 8>
	class TQueueList {
	public:
		typedef TQueue<T, K> Node;

	protected:
		Node* pushHead;
		Node* popHead; // popHead is always prior to pushHead

	public:
		TQueueList() {
			pushHead = popHead = new Node();
		}

		TQueueList(const TQueueList& rhs) {
			assert(rhs.Empty());
			pushHead = popHead = new Node();
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
				// free memory
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
			while (!pushHead->Push(t)) { // full
				if (pushHead->next == nullptr) {
					Node* p = new Node();
					bool success = p->Push(t); // Must success
					assert(success);

					pushHead->next = p;
					std::atomic_thread_fence(std::memory_order_release);
					pushHead = p;
					return;
				}

				pushHead = pushHead->next;
			}

			std::atomic_thread_fence(std::memory_order_release);
		}
#else
		template <class D>
		inline void Push(D&& t) {
			while (!pushHead->Push(std::forward<D>(t))) { // full
				if (pushHead->next == nullptr) {
					Node* p = new Node();
					bool success = p->Push(std::forward<D>(t)); // Must success
					assert(success);

					pushHead->next = p;
					std::atomic_thread_fence(std::memory_order_release);
					pushHead = p;
					return;
				}

				pushHead = pushHead->next;
			}

			std::atomic_thread_fence(std::memory_order_release);
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

		inline void Pop() {
			popHead->Pop();

			if (popHead->Empty() && popHead != pushHead) {
				Node* p = popHead;
				popHead = popHead->next;
				delete p;
			}
		}

		inline uint32_t GetPackCount(uint32_t alignment) const {
			uint32_t v = pushHead->GetPackCount(alignment);
			return v == 0 ? GetFullPackCount() : v;
		}

		inline T* Allocate(uint32_t count, uint32_t alignment) {
			T* address;
			while ((address = pushHead->Allocate(count, alignment)) == nullptr) {
				if (pushHead->next == nullptr) {
					Node* p = new Node();
					address = p->Allocate(count, alignment); // Must success
					assert(address != nullptr);

					pushHead->next = p;
					pushHead = p;
					return address;
				}

				pushHead = pushHead->next;
			}

			return address;
		}

		inline void Deallocate(uint32_t size, uint32_t alignment) {
			popHead->Deallocate(size, alignment);

			if (popHead->Empty() && popHead != pushHead) {
				Node* p = popHead;
				popHead = popHead->next;
				delete p;
			}
		}

		inline void Reset(uint32_t reserved) {
			Node* p = pushHead = popHead;
			p->Reset(); // always reserved
			uint32_t c = GetFullPackCount();
			Node* q = p;
			p = p->next;

			while (p != nullptr && c < reserved) {
				p->Reset();
				c += GetFullPackCount();
				q = p;
				p = p->next;
			}

			while (p != nullptr) {
				Node* t = p;
				p = p->next;
				delete t;
			}

			q->next = nullptr;
		}

		inline bool Empty() const {
			return popHead->Empty();
		}

		struct Iterator {
			Iterator(Node* n, typename Node::Iterator t) : p(n), it(t) {}
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
			typename Node::Iterator it;
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

		static inline uint32_t GetFullPackCount() {
			return Node::GetFullPackCount();
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

