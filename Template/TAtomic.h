#pragma once
#include "../PaintsNow.h"
#include <string>
#include <exception>
#include <stdexcept>
#include <algorithm>

#if defined(_MSC_VER)
#if defined(_M_IX86) || defined(_M_AMD64)
#include <emmintrin.h>

#if _MSC_VER <= 1200
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#include "../Interface/IType.h"
#include <windows.h>
#include <winbase.h>

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

inline void YieldThread() {
	if (!SwitchToThread()) {
		for (int i = 0; i < 16; i++) {
			_mm_pause();
		}
	}
}
#else

#include <atomic>
#include <thread>
inline void YieldThread() {
	std::this_thread::yield();
}

#endif

inline void YieldThreadFast() {
	_mm_pause();
}

#else
inline void YieldThread() {
	std::this_thread::yield();
}

inline void YieldThreadFast() {
	std::this_thread::yield();
}
#endif
#else
#include <atomic>
#include <thread>

inline void YieldThread() {
	std::this_thread::yield();
}

inline void YieldThreadFast() {
	timespec spec;
	spec.tv_sec = 0;
	spec.tv_nsec = 1;
	nanosleep(&spec, nullptr);
}
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
			T result = (T)value;
			atomic_thread_fence(order);
			return result;
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

		bool compare_exchange_strong(T& old, T u, std::memory_order order = std::memory_order_seq_cst) {
			T org = old;
			T result = (T)InterlockedCompareExchange((volatile long*)&value, (int32_t)u, (long)org);
			if (result == old) {
				return true;
			}

			old = result;
			return false;
		}

		bool compare_exchange_weak(T& old, T u, std::memory_order order = std::memory_order_seq_cst) {
			return compare_exchange_strong(old, u, order);
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

