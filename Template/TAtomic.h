#ifndef __TATOMIC_H__
#define __TATOMIC_H__

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

/*
__forceinline void MemoryBarrier() {
	LONG Barrier;
	__asm {
		xchg Barrier, eax
	}
}*/

#else
#include <atomic>
#include <thread>
#endif

namespace PaintsNow {
	// From Microsoft Visual C++ Header file.
	// implement std::atomic<> for Visual C++ 6.0
#if defined(_MSC_VER) && _MSC_VER <= 1200

	// only support 32 bit atomic operations ..
	template <class T>
	class TAtomic {
	public:
		TAtomic(int32_t v = 0) : value(v) {}

		void operator ++ (int) {
			InterlockedIncrement(&value);
		}

		int32_t operator ++ () {
			return InterlockedIncrement(&value);
		}

		void operator -- (int) {
			InterlockedDecrement(&value);
		}

		int32_t operator -- () {
			return InterlockedDecrement(&value);
		}

		void operator += (T arg) {
			InterlockedExchangeAdd(&value, (int32_t)arg);
		}

		void operator -= (T arg) {
			InterlockedExchangeAdd(&value, -(int32_t)arg);
		}

		void operator |= (T arg) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old | (int32_t)arg,
				Old) != Old);
		}

		void operator &= (T arg) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old & (int32_t)arg,
				Old) != Old);
		}

		void operator ^= (T arg) {
			LONG Old;

			do {
				Old = value;
			} while (InterlockedCompareExchange(&value,
				Old ^ (int32_t)arg,
				Old) != Old);
		}

		T fetch_add(T v, std::memory_order order = std::memory_order_release) {
			return (T)InterlockedExchangeAdd(&value, (int32_t)v);
		}

		T fetch_sub(T v, std::memory_order order = std::memory_order_release) {
			return (T)InterlockedExchangeAdd(&value, -(int32_t)v);
		}

		T load(std::memory_order order = std::memory_order_acquire) const {
			if (order != std::memory_order_relaxed) {
				MemoryBarrier();
			}

			return (T)value;
		}

		void store(T v, std::memory_order order = std::memory_order_release) {
			if (order == std::memory_order_seq_cst) {
				InterlockedExchange(&value, (int32_t)v);
			} else if (order != std::memory_order_relaxed) {
				MemoryBarrier();
				value = (int32_t)v;
			} else {
				value = (int32_t)v;
			}
		}

		int32_t operator & (T t) const {
			return value & (int32_t)t;
		}

		int32_t operator | (T t) const {
			return value | (int32_t)t;
		}

		int32_t operator ^ (T t) const {
			return value ^ (int32_t)t;
		}

		int32_t exchange(T t, std::memory_order order = std::memory_order_seq_cst) {
			return InterlockedExchange(&value, (int32_t)t);
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

#else
	template <class T>
	using TAtomic = std::atomic<T>;
#endif

	template <class T, class D>
	bool Verify(TAtomic<T>& var, D bits) {
		return (var & bits) == bits;
	}

	template <class T>
	inline T SpinLock(TAtomic<T>& section, T newValue = 1) {
		T ret;
		while (true) {
			while (section.load(std::memory_order_relaxed) == newValue) {
				YieldThreadFast();
			}

			if ((ret = section.exchange(newValue, std::memory_order_acquire)) != newValue) {
				break;
			}
		}
		
		return ret;
	}

	template <class T>
	inline void SpinUnLock(TAtomic<T>& section, T newValue = 0) {
		section.store(newValue, std::memory_order_release);
	}

#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T>
	class TStaticInitializer {
	public:
		TStaticInitializer() {
			static TAtomic<uint32_t> critical;
			SpinLock(critical);
			static T object;
			SpinUnLock(critical);
		}
	};
#else
	template <class T>
	class TStaticInitializer {
	public:
		TStaticInitializer() {
			// C++ 11 provides thread-safety for static variables.
			static T object;
		}
	};
#endif
}

#endif // __TATOMIC_H__