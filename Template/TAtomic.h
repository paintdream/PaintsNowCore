#pragma once
#include "../PaintsNow.h"
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include "../Interface/IThread.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#include "../Interface/IType.h"
#include <windows.h>
#include <winbase.h>
#include <emmintrin.h>

namespace std {
	enum memory_order {
		memory_order_relaxed,
		memory_order_consume,
		memory_order_acquire,
		memory_order_release,
		memory_order_acq_rel,
		memory_order_seq_cst
	};
}

#else
#include <atomic>
#include <thread>
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1200
namespace std {
	// From Microsoft Visual C++ Header file.
	// implement std::atomic<> for Visual C++ 6.0
	// only support 32 bit atomic operations ..
	inline void atomic_thread_fence(std::memory_order order) {
		if (order != std::memory_order_relaxed) {
			MemoryBarrier();
		}
	}

	template <class T>
	class atomic {
	public:
		atomic(int32_t v = 0) : value(v) {}
	
		int32_t fetch_add(T arg, std::memory_order order = std::memory_order_seq_cst) {
			return InterlockedExchangeAdd(&value, (int32_t)arg);
		}

		int32_t fetch_sub(T arg, std::memory_order order = std::memory_order_seq_cst) {
			return InterlockedExchangeAdd(&value, -(int32_t)arg);
		}

		int32_t fetch_or(T arg, std::memory_order order = std::memory_order_seq_cst) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old | (int32_t)arg,
				Old) != Old);

			return Old;
		}

		int32_t fetch_and(T arg, std::memory_order order = std::memory_order_seq_cst) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old & (int32_t)arg,
				Old) != Old);

			return Old;
		}

		int32_t fetch_xor(T arg, std::memory_order order = std::memory_order_seq_cst) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old ^ (int32_t)arg,
				Old) != Old);

			return Old;
		}

		T load(std::memory_order order = std::memory_order_acquire) const {
			T m = (T)value;
			atomic_thread_fence(order);
			return m;
		}

		void store(T v, std::memory_order order = std::memory_order_release) {
			if (order != std::memory_order_seq_cst) {
				atomic_thread_fence(order);
				value = (int32_t)v;
			} else {
				exchange(v, std::memory_order_seq_cst);
			}
		}

		T operator & (T t) const {
			return (T)(load(std::memory_order_acquire) & (int32_t)t);
		}

		T operator | (T t) const {
			return (T)(load(std::memory_order_acquire) | (int32_t)t);
		}

		T operator ^ (T t) const {
			return (T)(load(std::memory_order_acquire) ^ (int32_t)t);
		}

		T exchange(T t, std::memory_order order = std::memory_order_seq_cst) {
			return (T)InterlockedExchange(&value, (int32_t)t);
		}

		bool compare_exchange_weak(T& old, T u, std::memory_order order = std::memory_order_seq_cst) {
			T org = old;
			return (old = (T)InterlockedCompareExchange((volatile long*)&value, (int32_t)u, (long)org)) == org;
		}

		bool compare_exchange_strong(T& old, T u, std::memory_order order = std::memory_order_seq_cst) {
			T org = old;
			return (old = (T)InterlockedCompareExchange((volatile long*)&value, (int32_t)u, (long)org)) == org;
		}

	private:
		volatile LONG value;
	};
}
#endif

namespace PaintsNow {
	template <class T>
	inline T SpinLock(std::atomic<T>& section, T newValue = 1) {
		T ret;
		while (true) {
			while (section.load(std::memory_order_acquire) == newValue) {
				YieldThreadFast();
			}

			if ((ret = section.exchange(newValue, std::memory_order_acquire)) != newValue) {
				break;
			}
		}
		
		return ret;
	}

	template <class T>
	inline void SpinUnLock(std::atomic<T>& section, T newValue = 0) {
		section.store(newValue, std::memory_order_release);
	}

#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T>
	class TSingleton {
	public:
		static T& Get() {
			static std::atomic<uint32_t> critical;
			static T* ptr = nullptr;
			if (ptr == nullptr) { // x86 is strongly ordered
				SpinLock(critical);
				if (ptr == nullptr) {
					static T object;
					ptr = &object;
				}

				SpinUnLock(critical); // unlock indicates std::memory_order_release
			}

			return *ptr;
		}
	};
#else
	template <class T>
	class TSingleton {
	public:
		static T& Get() {
			// C++ 11 provides thread-safety for static variables.
			static T object;
			return object;
		}
	};
#endif
}

