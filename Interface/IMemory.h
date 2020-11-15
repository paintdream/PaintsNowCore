// IMemory -- Basic memory allocator interface
// By PaintDream (paintdream@paintdream.com)
// 2014-12-3
//

#pragma once
#include "../PaintsNow.h"
#include "../Template/TAtomic.h"
#include <malloc.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

namespace PaintsNow {
	namespace IMemory {
		template <class T>
		struct ObjectLeakGuard {
#ifdef _DEBUG
			struct Finalizer {
				Finalizer(std::atomic<int32_t>& c) : counter(c) {}
				~Finalizer() {
					assert(counter.load(std::memory_order_relaxed) == 0);
				}

				std::atomic<int32_t>& counter;
			};

			ObjectLeakGuard() {
				++GetCounter();
			}

			~ObjectLeakGuard() {
				--GetCounter();
			}

			static std::atomic<int32_t>& GetCounter() {
				static std::atomic<int32_t> counter;
				static Finalizer finalizer(counter);
				return counter;
			}
#endif
		};

		static inline void PrefetchRead(const void* address) {
#ifdef _MSC_VER
			_mm_prefetch(reinterpret_cast<const char*>(address), _MM_HINT_NTA);
#else
			__builtin_prefetch(address, 0, 0);
#endif
		}

		static inline void PrefetchReadLocal(const void* address) {
#ifdef _MSC_VER
			_mm_prefetch(reinterpret_cast<const char*>(address), _MM_HINT_T0);
#else
			__builtin_prefetch(address, 0, 3);
#endif
		}

		static inline void PrefetchWrite(void* address) {
#ifdef _MSC_VER
			_mm_prefetch(reinterpret_cast<char*>(address), _MM_HINT_NTA);
#else
			__builtin_prefetch(address, 1, 0);
#endif
		}

		static inline void PrefetchWriteLocal(void* address) {
#ifdef _MSC_VER
			_mm_prefetch(reinterpret_cast<char*>(address), _MM_HINT_T0);
#else
			__builtin_prefetch(address, 1, 3);
#endif
		}

		static inline void* AllocAligned(size_t size, size_t alignment) {
#ifdef _MSC_VER
			return _aligned_malloc(size, alignment);
#else
			return memalign(alignment, size);
#endif
		}

		static inline void FreeAligned(void* data) {
#ifdef _MSC_VER
			_aligned_free(data);
#else
			free(data);
#endif
		}

		enum BREAK_TYPE { EXECUTE, READWRITE, WRITE };
		void SetHardwareBreakpoint(void* address, size_t length, BREAK_TYPE type, size_t slot);
	};
}

