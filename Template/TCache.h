// TBuffer -- Basic buffer structure
// By PaintDream (paintdream@paintdream.com)
// 2014-11-30
//

#pragma once
#include "../PaintsNow.h"
#include "TBuffer.h"
#include "TQueue.h"

namespace PaintsNow {
	template <class T, size_t K>
	class TCache {
	public:
		TBuffer<T> New(uint32_t size, uint32_t alignment = 16) {
			uint32_t pack = allocator.GetPackCount(alignment);
			static_assert(alignof(TBuffer<T>) % sizeof(T) == 0, "TBuffer<T> must be aligned at least sizeof(T).");
			const uint32_t headCount = sizeof(TBuffer<T>) / sizeof(T);

			if (size > pack) {
				T* slice = allocator.Allocate(pack, alignment);
				TBuffer<T> head = TBuffer<T>::View(slice, pack);
				TBuffer<T>* p = &head;
				size -= pack;
				pack = allocator.GetFullPackCount() - headCount;
				alignment = Math::Max(alignment, (uint32_t)(alignof(TBuffer<T>) / sizeof(T)));

				while (size != 0) {
					uint32_t allocCount = Math::Min(size, pack);
					slice = allocator.Allocate(allocCount + headCount, alignment);
					TBuffer<T>* next = new (slice) TBuffer<T>();
					*next = TBuffer<T>::View(slice + headCount, pack);
					size -= allocCount;

					p->Append(*next);
					p = next;
				}

				return head;
			} else {
				return TBuffer<T>::View(allocator.Allocate(size, alignment), size);
			}
		}

		void Link(TBuffer<T>& from, const TBuffer<T>& to) {
			if (from.Empty()) {
				from = to;
			} else {
				assert(from.IsViewStorage() && to.IsViewStorage());
				TBuffer<T> storage = New(sizeof(TBuffer<T>) / sizeof(T), alignof(TBuffer<T>) / sizeof(T));
				from.Append(*new (storage.GetData()) TBuffer<T>(to));
			}
		}

		void Reset() {
			allocator.Reset(~(uint32_t)0);
		}

		void Clear() {
			allocator.Reset(0);
		}

	protected:
		TQueueList<T, K> allocator;
	};

	typedef TCache<uint8_t, 12> BytesCache;
}
