// TAlgorithm.h -- Basic algorithms
// By PaintDream (paintdream@paintdream.com)
// 2014-12-2
//

#ifndef __TALGORITHM_H__
#define __TALGORITHM_H__

#include "../PaintsNow.h"
#include <cstring>
#include <algorithm>
#include <functional>

namespace PaintsNow {
	template <class T>
	T Max(T a, T b) {
		return a > b ? a : b;
	}

	template <class T>
	T Min(T a, T b) {
		return a < b ? a : b;
	}

	template <class T>
	T Clamp(T x, T a, T b) {
		return x < a ? a : x > b ? b : x;
	}

	template <class T>
	T AllMin(const T& lhs, const T& rhs) {
		T result;
		for (size_t i = 0; i < T::size; i++) {
			result[i] = Min(lhs[i], rhs[i]);
		}

		return result;
	}

	template <class T>
	T AllMax(const T& lhs, const T& rhs) {
		T result;
		for (size_t i = 0; i < T::size; i++) {
			result[i] = Max(lhs[i], rhs[i]);
		}

		return result;
	}

	template <class T>
	T AllClamp(const T& v, const T& lhs, const T& rhs) {
		T result;
		for (size_t i = 0; i < T::size; i++) {
			result[i] = Clamp(v[i], lhs[i], rhs[i]);
		}

		return result;
	}

	template <class T, class D>
	T Interpolate(const T& a, const T& b, D alpha) {
		return a * (1 - alpha) + b * alpha;
	}

	template <class T>
	T Alignment(T a) {
		return a & (~a + 1);
	}

	template <class T>
	T LogAlignmentTop(T t) {
		T z = 0;
		while (t != 0) {
			t >>= 1;
			z++;
		}

		return z;
	}

	template <class T>
	T AlignmentTop(T a) {
		T z = 1, t = a;
		while ((t >>= 1) != 0) {
			z <<= 1;
		}

		return z;
	}

	template <class T>
	T AlignmentRange(T a, T b) {
		T top = AlignmentTop(a ^ b);
		return (a & ~(top - 1)) | top;
	}

	template <class T>
	T SiteCount(T a, T b) {
		T c = AlignmentRange(a, b);
		T count = 0;
		if (a != 0) {
			while (a < c) {
				a += Alignment(a);
				count++;
			}
		} else {
			count++;
		}

		if (b != 0) {
			while (b > c) {
				b -= Alignment(b);
				count++;
			}
		}

		return count;
	}

	template <class T>
	T ReverseBytes(const T& src) {
		T dst = src;
		char* ptr = reinterpret_cast<char*>(&dst);
		for (size_t i = 0; i < sizeof(T) / 2; i++) {
			std::swap(ptr[i], ptr[sizeof(T) - i - 1]);
		}

		return dst;
	}
}


namespace std {
	template <class I, class D, class P>
	I binary_find(I begin, I end, const D& value, const P& pred) {
		I it = lower_bound(begin, end, value, pred);
		return it != end && !pred(value, *it) ? it : end;
	}

	template <class I, class D>
	I binary_find(I begin, I end, const D& value) {
		I it = lower_bound(begin, end, value);
		return it != end && !(value < *it) ? it : end;
	}

#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T, class D, class P>
	typename T::iterator binary_insert(T& container, rvalue<D> value, const P& pred) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value, pred);
		typename T::iterator ip = it;
		if (it != container.begin() && !pred(*--ip, value)) {
			*ip = std::move(value);
			return ip;
		} else {
			return container.insert(it, value);
		}
	}

	template <class T, class D, class P>
	typename T::iterator binary_insert(T& container, const D& value, const P& pred) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value, pred);
		typename T::iterator ip = it;
		if (it != container.begin() && !pred(*--ip, value)) {
			*ip = value;
			return ip;
		} else {
			return container.insert(it, value);
		}
	}

	template <class T, class D>
	typename T::iterator binary_insert(T& container, const D& value) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value);
		typename T::iterator ip = it;
		if (it != container.begin() && !(*--ip < value)) {
			*ip = value;
			return ip;
		} else {
			return container.insert(it, value);
		}
	}
#else
	template <class T, class D, class P>
	typename T::iterator binary_insert(T& container, D&& value, const P& pred) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value, pred);
		typename T::iterator ip = it;
		if (it != container.begin() && !pred(*--ip, value)) {
			*ip = std::forward<D>(value);
			return ip;
		} else {
			return container.insert(it, std::forward<D>(value));
		}
	}

	template <class T, class D>
	typename T::iterator binary_insert(T& container, D&& value) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value);
		typename T::iterator ip = it;
		if (it != container.begin() && !(*--ip < value)) {
			*ip = std::forward<D>(value);
			return ip;
		} else {
			return container.insert(it, std::forward<D>(value));
		}
	}
#endif
	template <class T, class D, class P>
	bool binary_erase(T& container, const D& value, const P& pred) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value, pred);
		typename T::iterator ip = it;
		if (it != container.begin() && !pred(*--ip, value)) {
			container.erase(ip);
			return true;
		} else {
			return false;
		}
	}

	template <class T, class D>
	bool binary_erase(T& container, const D& value) {
		typename T::iterator it = upper_bound(container.begin(), container.end(), value);
		typename T::iterator ip = it;
		if (it != container.begin() && !(*--ip < value)) {
			container.erase(ip);
			return true;
		} else {
			return false;
		}
	}
}

#endif // __TALGORITHM_H__
