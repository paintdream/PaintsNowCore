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

		static inline uint32_t GetFullPackCount() {
			return TQueueList<T, K>::GetFullPackCount();
		}

		T* NewLinear(uint32_t size, uint32_t alignment = 16) {
			return allocator.Allocate(size, alignment);
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

	template <class T, class D = uint8_t, size_t K = 12>
	struct TCacheAllocator {
		typedef T value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef std::true_type propagate_on_container_move_assignment;
		template <class M>
		struct rebind { typedef TCacheAllocator<M, D, K> other; };
		typedef std::false_type is_always_equal;

		typedef TCache<D, K> Allocator;
		Allocator* allocator;

		TCacheAllocator(Allocator* alloc) : allocator(alloc) {}
		TCacheAllocator(const TCacheAllocator& al) : allocator(al.allocator) {}

#if defined(_MSC_VER) && _MSC_VER > 1200
		// maybe a bug of VC (Debug Build), just add these lines to make compiler happy
		template <class M>
		TCacheAllocator(const TCacheAllocator<M, D, K>& rhs) : allocator(rhs.allocator) {}
#endif

		template <class M>
		bool operator == (const M& rhs) const noexcept {
			return allocator == rhs.allocator;
		}

		template <class M>
		bool operator != (const M& rhs) const noexcept {
			return allocator != rhs.allocator;
		}

		pointer address(reference x) const noexcept {
			return &x;
		};

		const_pointer address(const_reference x) const noexcept {
			return &x;
		}

		size_type max_size() const noexcept {
			return ~(size_type)0 / sizeof(T);
		}

		void construct(pointer p, const_reference val) {
			new ((void*)p) T(val);
		}

#if !defined(_MSC_VER) || _MSC_VER > 1200
		template <class M, class... Args>
		void construct(M* p, Args&&... args) {
			new ((void*)p) M(std::forward<Args>(args)...);
		}
#endif
		template <class M>
		void destroy(M* p) {
			((M*)p)->~M();
		}

		forceinline pointer allocate(size_type n, const void* hint = nullptr) {
			assert(allocator != nullptr);
			static_assert(sizeof(T) % sizeof(D) == 0, "Must be aligned.");
			uint32_t count = safe_cast<uint32_t>(n * sizeof(T) / sizeof(D));
			if (count <= Allocator::GetFullPackCount()) {
				return reinterpret_cast<pointer>(allocator->NewLinear(count, safe_cast<uint32_t>(alignof(T))));
			} else {
				return reinterpret_cast<pointer>(::operator new(sizeof(T) * n));
			}
		}

		forceinline void deallocate(T* p, size_t n) {
			uint32_t count = safe_cast<uint32_t>(n * sizeof(T) / sizeof(D));
			if (count <= Allocator::GetFullPackCount()) {
				// do not deallocate in cache allocator.
			} else {
				::operator delete(p);
			}
		}
	};
}
