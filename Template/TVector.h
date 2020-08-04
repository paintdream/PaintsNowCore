// TVector -- Basic abstract template vector
// By PaintDream (paintdream@paintdream.com)
// 2014-11-30
//

#ifndef __TVECTOR_H__
#define __TVECTOR_H__


#include "../PaintsNow.h"
#include "TAlgorithm.h"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace PaintsNow {
	template <class T, size_t n>
	class TVector {
	public:
		typedef T type;
		TVector(const std::pair<TVector<T, n / 2>, TVector<T, n / 2> >& p) {
			memcpy(data, &p.first.data[0], n * sizeof(T) / 2);
			memcpy(data + n / 2, &p.second.data[0], n * sizeof(T) / 2);
		}

		TVector(T* v = nullptr) {
			if (v != nullptr) {
				std::copy(v, v + n, data);
			}
		}

		enum { size = n };
		inline operator const T* () const {
			return data;
		}

		inline operator T* () {
			return data;
		}

		inline bool operator == (const TVector& rhs) const {
			for (size_t i = 0; i < n; i++) {
				if (data[i] != rhs.data[i]) return false;
			}

			return true;
		}

		inline bool operator != (const TVector& rhs) const {
			for (size_t i = 0; i < n; i++) {
				if (data[i] != rhs.data[i]) return true;
			}

			return false;
		}

		template <class D>
		inline const T operator [] (D d) const {
			return data[d];
		}

		template <class D>
		inline T& operator [] (D d) {
			return data[d];
		}

		template <class D>
		inline TVector<T, n>& Foreach(D op) {
			for (size_t i = 0; i < n; i++) {
				op(data[i]);
			}

			return *this;
		}

		inline T Length() const {
			return (T)sqrt(SquareLength());
		}

		inline TVector<T, n> operator - () const {
			TVector<T, n> ret;
			for (size_t i = 0; i < n; i++) {
				ret[i] = - data[i];
			}

			return ret;
		}

		inline TVector<T, n>& operator += (const TVector<T, n>& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] += data[i];
			}

			return ret;
		}

		inline TVector<T, n>& operator -= (const TVector<T, n>& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] -= data[i];
			}

			return ret;
		}

		TVector<T, n>& operator *= (const TVector<T, n>& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] *= data[i];
			}

			return ret;
		}

		inline TVector<T, n>& operator /= (const TVector<T, n>& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] /= data[i];
			}

			return ret;
		}

		inline TVector<T, n>& operator *= (const T& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] *= data;
			}

			return ret;
		}

		inline TVector<T, n>& operator /= (const T& data) {
			TVector<T, n>& ret = *this;
			for (size_t i = 0; i < n; i++) {
				ret[i] /= data;
			}

			return ret;
		}

		inline bool operator < (const TVector<T, n>& rhs) const {
			const TVector<T, n>& lhs = *this;
			for (size_t i = 0; i < n; i++) {
				if (lhs[i] < rhs[i])
					return true;
				else if (rhs[i] < lhs[i])
					return false;
			}

			return false;
		}

		inline T SquareLength() const {
			T t(0);
			for (size_t i = 0; i < n; i++) {
				t += data[i] * data[i];
			}
			
			return t;
		}

		inline TVector<T, n>& Normalize() {
			T length = Length();
			for (size_t j = 0; j < n; j++) {
				data[j] /= length;
			}

			return *this;
		}

		T data[n];
	};

#define VISIT(X, index) \
	inline const T X() const { return (*this)[index]; } \
	inline T& X() { return (*this)[index]; }

// Fake visit, just make compiler happy
#define SWIZZLE2(X, Y) \
	inline TType2<T> _##X##Y() const { return TType2<T>(X(), Y()); } \

#define SWIZZLE3(X, Y, Z) \
	inline TType3<T> _##X##Y##Z() const { return TType3<T>(X(), Y(), Z()); } \

#define SWIZZLE4(X, Y, Z, W) \
	inline TType4<T> _##X##Y##Z##W() const { return TType4<T>(X(), Y(), Z(), W()); } \

#define SWIZZLE4_FULL(X, Y, Z, W) \
	inline TType4<T> _##X##X##X##X() const { return TType4<T>(X(), X(), X(), X()); } \
	inline TType4<T> _##X##X##X##Y() const { return TType4<T>(X(), X(), X(), Y()); } \
	inline TType4<T> _##X##X##X##Z() const { return TType4<T>(X(), X(), X(), Z()); } \
	inline TType4<T> _##X##X##X##W() const { return TType4<T>(X(), X(), X(), W()); } \
	\
	inline TType4<T> _##X##X##Y##X() const { return TType4<T>(X(), X(), Y(), X()); } \
	inline TType4<T> _##X##X##Y##Y() const { return TType4<T>(X(), X(), Y(), Y()); } \
	inline TType4<T> _##X##X##Y##Z() const { return TType4<T>(X(), X(), Y(), Z()); } \
	inline TType4<T> _##X##X##Y##W() const { return TType4<T>(X(), X(), Y(), W()); } \
	\
	inline TType4<T> _##X##X##Z##X() const { return TType4<T>(X(), X(), Z(), X()); } \
	inline TType4<T> _##X##X##Z##Y() const { return TType4<T>(X(), X(), Z(), Y()); } \
	inline TType4<T> _##X##X##Z##Z() const { return TType4<T>(X(), X(), Z(), Z()); } \
	inline TType4<T> _##X##X##Z##W() const { return TType4<T>(X(), X(), Z(), W()); } \
	\
	inline TType4<T> _##X##X##W##X() const { return TType4<T>(X(), X(), W(), X()); } \
	inline TType4<T> _##X##X##W##Y() const { return TType4<T>(X(), X(), W(), Y()); } \
	inline TType4<T> _##X##X##W##Z() const { return TType4<T>(X(), X(), W(), Z()); } \
	inline TType4<T> _##X##X##W##W() const { return TType4<T>(X(), X(), W(), W()); } \
	\
	\
	inline TType4<T> _##X##Y##X##X() const { return TType4<T>(X(), Y(), X(), X()); } \
	inline TType4<T> _##X##Y##X##Y() const { return TType4<T>(X(), Y(), X(), Y()); } \
	inline TType4<T> _##X##Y##X##Z() const { return TType4<T>(X(), Y(), X(), Z()); } \
	inline TType4<T> _##X##Y##X##W() const { return TType4<T>(X(), Y(), X(), W()); } \
	\
	inline TType4<T> _##X##Y##Y##X() const { return TType4<T>(X(), Y(), Y(), X()); } \
	inline TType4<T> _##X##Y##Y##Y() const { return TType4<T>(X(), Y(), Y(), Y()); } \
	inline TType4<T> _##X##Y##Y##Z() const { return TType4<T>(X(), Y(), Y(), Z()); } \
	inline TType4<T> _##X##Y##Y##W() const { return TType4<T>(X(), Y(), Y(), W()); } \
	\
	inline TType4<T> _##X##Y##Z##X() const { return TType4<T>(X(), Y(), Z(), X()); } \
	inline TType4<T> _##X##Y##Z##Y() const { return TType4<T>(X(), Y(), Z(), Y()); } \
	inline TType4<T> _##X##Y##Z##Z() const { return TType4<T>(X(), Y(), Z(), Z()); } \
	inline TType4<T> _##X##Y##Z##W() const { return TType4<T>(X(), Y(), Z(), W()); } \
	\
	inline TType4<T> _##X##Y##W##X() const { return TType4<T>(X(), Y(), W(), X()); } \
	inline TType4<T> _##X##Y##W##Y() const { return TType4<T>(X(), Y(), W(), Y()); } \
	inline TType4<T> _##X##Y##W##Z() const { return TType4<T>(X(), Y(), W(), Z()); } \
	inline TType4<T> _##X##Y##W##W() const { return TType4<T>(X(), Y(), W(), W()); } \
	\
	\
	inline TType4<T> _##X##Z##X##X() const { return TType4<T>(X(), Z(), X(), X()); } \
	inline TType4<T> _##X##Z##X##Y() const { return TType4<T>(X(), Z(), X(), Y()); } \
	inline TType4<T> _##X##Z##X##Z() const { return TType4<T>(X(), Z(), X(), Z()); } \
	inline TType4<T> _##X##Z##X##W() const { return TType4<T>(X(), Z(), X(), W()); } \
	\
	inline TType4<T> _##X##Z##Y##X() const { return TType4<T>(X(), Z(), Y(), X()); } \
	inline TType4<T> _##X##Z##Y##Y() const { return TType4<T>(X(), Z(), Y(), Y()); } \
	inline TType4<T> _##X##Z##Y##Z() const { return TType4<T>(X(), Z(), Y(), Z()); } \
	inline TType4<T> _##X##Z##Y##W() const { return TType4<T>(X(), Z(), Y(), W()); } \
	\
	inline TType4<T> _##X##Z##Z##X() const { return TType4<T>(X(), Z(), Z(), X()); } \
	inline TType4<T> _##X##Z##Z##Y() const { return TType4<T>(X(), Z(), Z(), Y()); } \
	inline TType4<T> _##X##Z##Z##Z() const { return TType4<T>(X(), Z(), Z(), Z()); } \
	inline TType4<T> _##X##Z##Z##W() const { return TType4<T>(X(), Z(), Z(), W()); } \
	\
	inline TType4<T> _##X##Z##W##X() const { return TType4<T>(X(), Z(), W(), X()); } \
	inline TType4<T> _##X##Z##W##Y() const { return TType4<T>(X(), Z(), W(), Y()); } \
	inline TType4<T> _##X##Z##W##Z() const { return TType4<T>(X(), Z(), W(), Z()); } \
	inline TType4<T> _##X##Z##W##W() const { return TType4<T>(X(), Z(), W(), W()); } \
	\
	\
	inline TType4<T> _##X##W##X##X() const { return TType4<T>(X(), W(), X(), X()); } \
	inline TType4<T> _##X##W##X##Y() const { return TType4<T>(X(), W(), X(), Y()); } \
	inline TType4<T> _##X##W##X##Z() const { return TType4<T>(X(), W(), X(), Z()); } \
	inline TType4<T> _##X##W##X##W() const { return TType4<T>(X(), W(), X(), W()); } \
	\
	inline TType4<T> _##X##W##Y##X() const { return TType4<T>(X(), W(), Y(), X()); } \
	inline TType4<T> _##X##W##Y##Y() const { return TType4<T>(X(), W(), Y(), Y()); } \
	inline TType4<T> _##X##W##Y##Z() const { return TType4<T>(X(), W(), Y(), Z()); } \
	inline TType4<T> _##X##W##Y##W() const { return TType4<T>(X(), W(), Y(), W()); } \
	\
	inline TType4<T> _##X##W##Z##X() const { return TType4<T>(X(), W(), Z(), X()); } \
	inline TType4<T> _##X##W##Z##Y() const { return TType4<T>(X(), W(), Z(), Y()); } \
	inline TType4<T> _##X##W##Z##Z() const { return TType4<T>(X(), W(), Z(), Z()); } \
	inline TType4<T> _##X##W##Z##W() const { return TType4<T>(X(), W(), Z(), W()); } \
	\
	inline TType4<T> _##X##W##W##X() const { return TType4<T>(X(), W(), W(), X()); } \
	inline TType4<T> _##X##W##W##Y() const { return TType4<T>(X(), W(), W(), Y()); } \
	inline TType4<T> _##X##W##W##Z() const { return TType4<T>(X(), W(), W(), Z()); } \
	inline TType4<T> _##X##W##W##W() const { return TType4<T>(X(), W(), W(), W()); } \
	\

	template <class T>
	struct TType3;

	template <class T>
	struct TType4;

#if defined(_MSC_VER) && _MSC_VER <= 1200
#define EXPLICIT
#else
#define EXPLICIT explicit
#endif

	template <class T>
	struct TType2 : public TVector<T, 2> {
		TType2() {}
		TType2(const std::pair<TVector<T, 1>, TVector<T, 1> >& p) : TVector<T, 2>(p) {}
		TType2(const TVector<T, 2>& v) : TVector<T, 2>(v) {}
		TType2(T xx, T yy) { x() = xx; y() = yy; }

		EXPLICIT operator float() const {
			return x();
		}

		VISIT(x, 0);
		VISIT(y, 1);
		VISIT(s, 0);
		VISIT(t, 1);

#ifdef USE_SWIZZLE
		SWIZZLE2(x, y);
		SWIZZLE2(y, x);

		SWIZZLE3(x, x, x);
		SWIZZLE3(x, x, y);
		SWIZZLE3(x, y, x);
		SWIZZLE3(x, y, y);

		SWIZZLE3(y, x, x);
		SWIZZLE3(y, x, y);
		SWIZZLE3(y, y, x);
		SWIZZLE3(y, y, y);

		SWIZZLE4(x, x, x, x);
		SWIZZLE4(x, x, x, y);
		SWIZZLE4(x, x, y, x);
		SWIZZLE4(x, x, y, y);

		SWIZZLE4(x, y, x, x);
		SWIZZLE4(x, y, x, y);
		SWIZZLE4(x, y, y, x);
		SWIZZLE4(x, y, y, y);

		SWIZZLE4(y, x, x, x);
		SWIZZLE4(y, x, x, y);
		SWIZZLE4(y, x, y, x);
		SWIZZLE4(y, x, y, y);

		SWIZZLE4(y, y, x, x);
		SWIZZLE4(y, y, x, y);
		SWIZZLE4(y, y, y, x);
		SWIZZLE4(y, y, y, y);
#endif
	};

	template <class T>
	struct TType3 : public TVector<T, 3> {
		TType3() {}
		TType3(const TVector<T, 3>& v) : TVector<T, 3>(v) {}
		TType3(T xx, T yy, T zz) { x() = xx; y() = yy; z() = zz; }

		EXPLICIT operator float() const {
			return x();
		}

		EXPLICIT operator TType2<T>() const {
			return TType2<T>(x(), y());
		}

		VISIT(x, 0);
		VISIT(y, 1);
		VISIT(z, 2);

		VISIT(r, 0);
		VISIT(g, 1);
		VISIT(b, 2);

#ifdef USE_SWIZZLE
		SWIZZLE2(x, x);
		SWIZZLE2(x, y);
		SWIZZLE2(x, z);
		SWIZZLE2(y, x);
		SWIZZLE2(y, y);
		SWIZZLE2(y, z);
		SWIZZLE2(z, x);
		SWIZZLE2(z, y);
		SWIZZLE2(z, z);

		SWIZZLE3(x, x, x);
		SWIZZLE3(x, x, y);
		SWIZZLE3(x, x, z);

		SWIZZLE3(x, y, x);
		SWIZZLE3(x, y, y);
		SWIZZLE3(x, y, z);

		SWIZZLE3(x, z, x);
		SWIZZLE3(x, z, y);
		SWIZZLE3(x, z, z);

		SWIZZLE3(y, x, x);
		SWIZZLE3(y, x, y);
		SWIZZLE3(y, x, z);

		SWIZZLE3(y, y, x);
		SWIZZLE3(y, y, y);
		SWIZZLE3(y, y, z);

		SWIZZLE3(y, z, x);
		SWIZZLE3(y, z, y);
		SWIZZLE3(y, z, z);

		SWIZZLE3(z, x, x);
		SWIZZLE3(z, x, y);
		SWIZZLE3(z, x, z);

		SWIZZLE3(z, y, x);
		SWIZZLE3(z, y, y);
		SWIZZLE3(z, y, z);

		SWIZZLE3(z, z, x);
		SWIZZLE3(z, z, y);
		SWIZZLE3(z, z, z);

		// 4
		SWIZZLE4(x, x, x, x);
		SWIZZLE4(x, x, x, y);
		SWIZZLE4(x, x, x, z);

		SWIZZLE4(x, x, y, x);
		SWIZZLE4(x, x, y, y);
		SWIZZLE4(x, x, y, z);

		SWIZZLE4(x, x, z, x);
		SWIZZLE4(x, x, z, y);
		SWIZZLE4(x, x, z, z);

		SWIZZLE4(x, y, x, x);
		SWIZZLE4(x, y, x, y);
		SWIZZLE4(x, y, x, z);

		SWIZZLE4(x, y, y, x);
		SWIZZLE4(x, y, y, y);
		SWIZZLE4(x, y, y, z);

		SWIZZLE4(x, y, z, x);
		SWIZZLE4(x, y, z, y);
		SWIZZLE4(x, y, z, z);

		SWIZZLE4(x, z, x, x);
		SWIZZLE4(x, z, x, y);
		SWIZZLE4(x, z, x, z);

		SWIZZLE4(x, z, y, x);
		SWIZZLE4(x, z, y, y);
		SWIZZLE4(x, z, y, z);

		SWIZZLE4(x, z, z, x);
		SWIZZLE4(x, z, z, y);
		SWIZZLE4(x, z, z, z);

		SWIZZLE4(y, x, x, x);
		SWIZZLE4(y, x, x, y);
		SWIZZLE4(y, x, x, z);

		SWIZZLE4(y, x, y, x);
		SWIZZLE4(y, x, y, y);
		SWIZZLE4(y, x, y, z);

		SWIZZLE4(y, x, z, x);
		SWIZZLE4(y, x, z, y);
		SWIZZLE4(y, x, z, z);

		SWIZZLE4(y, y, x, x);
		SWIZZLE4(y, y, x, y);
		SWIZZLE4(y, y, x, z);

		SWIZZLE4(y, y, y, x);
		SWIZZLE4(y, y, y, y);
		SWIZZLE4(y, y, y, z);

		SWIZZLE4(y, y, z, x);
		SWIZZLE4(y, y, z, y);
		SWIZZLE4(y, y, z, z);

		SWIZZLE4(y, z, x, x);
		SWIZZLE4(y, z, x, y);
		SWIZZLE4(y, z, x, z);

		SWIZZLE4(y, z, y, x);
		SWIZZLE4(y, z, y, y);
		SWIZZLE4(y, z, y, z);

		SWIZZLE4(y, z, z, x);
		SWIZZLE4(y, z, z, y);
		SWIZZLE4(y, z, z, z);

		SWIZZLE4(z, x, x, x);
		SWIZZLE4(z, x, x, y);
		SWIZZLE4(z, x, x, z);

		SWIZZLE4(z, x, y, x);
		SWIZZLE4(z, x, y, y);
		SWIZZLE4(z, x, y, z);

		SWIZZLE4(z, x, z, x);
		SWIZZLE4(z, x, z, y);
		SWIZZLE4(z, x, z, z);

		SWIZZLE4(z, y, x, x);
		SWIZZLE4(z, y, x, y);
		SWIZZLE4(z, y, x, z);

		SWIZZLE4(z, y, y, x);
		SWIZZLE4(z, y, y, y);
		SWIZZLE4(z, y, y, z);

		SWIZZLE4(z, y, z, x);
		SWIZZLE4(z, y, z, y);
		SWIZZLE4(z, y, z, z);

		SWIZZLE4(z, z, x, x);
		SWIZZLE4(z, z, x, y);
		SWIZZLE4(z, z, x, z);

		SWIZZLE4(z, z, y, x);
		SWIZZLE4(z, z, y, y);
		SWIZZLE4(z, z, y, z);

		SWIZZLE4(z, z, z, x);
		SWIZZLE4(z, z, z, y);
		SWIZZLE4(z, z, z, z);
#endif
	};

	template <class T>
	struct TType4 : public TVector<T, 4> {
		TType4() {}
		TType4(const std::pair<TVector<T, 2>, TVector<T, 2> >& p) : TVector<T, 4>(p) {}
		TType4(const TVector<T, 4>& v) : TVector<T, 4>(v) {}
		TType4(T xx, T yy, T zz, T ww) { x() = xx; y() = yy; z() = zz; w() = ww; }

		EXPLICIT operator float() const {
			return x();
		}

		EXPLICIT operator TType2<T>() const {
			return TType2<T>(x(), y());
		}
		
		EXPLICIT operator TType3<T>() const {
			return TType3<T>(x(), y(), z());
		}

		VISIT(x, 0);
		VISIT(y, 1);
		VISIT(z, 2);
		VISIT(w, 3);

		VISIT(r, 0);
		VISIT(g, 1);
		VISIT(b, 2);
		VISIT(a, 3);

#ifdef USE_SWIZZLE
		SWIZZLE2(x, x);
		SWIZZLE2(x, y);
		SWIZZLE2(x, z);
		SWIZZLE2(x, w);

		SWIZZLE2(y, x);
		SWIZZLE2(y, y);
		SWIZZLE2(y, z);
		SWIZZLE2(y, w);

		SWIZZLE2(z, x);
		SWIZZLE2(z, y);
		SWIZZLE2(z, z);
		SWIZZLE2(z, w);

		SWIZZLE2(w, x);
		SWIZZLE2(w, y);
		SWIZZLE2(w, z);
		SWIZZLE2(w, w);

		SWIZZLE3(x, x, x);
		SWIZZLE3(x, x, y);
		SWIZZLE3(x, x, z);
		SWIZZLE3(x, x, w);

		SWIZZLE3(x, y, x);
		SWIZZLE3(x, y, y);
		SWIZZLE3(x, y, z);
		SWIZZLE3(x, y, w);

		SWIZZLE3(x, z, x);
		SWIZZLE3(x, z, y);
		SWIZZLE3(x, z, z);
		SWIZZLE3(x, z, w);

		SWIZZLE3(x, w, x);
		SWIZZLE3(x, w, y);
		SWIZZLE3(x, w, z);
		SWIZZLE3(x, w, w);

		SWIZZLE3(y, x, x);
		SWIZZLE3(y, x, y);
		SWIZZLE3(y, x, z);
		SWIZZLE3(y, x, w);

		SWIZZLE3(y, y, x);
		SWIZZLE3(y, y, y);
		SWIZZLE3(y, y, z);
		SWIZZLE3(y, y, w);

		SWIZZLE3(y, z, x);
		SWIZZLE3(y, z, y);
		SWIZZLE3(y, z, z);
		SWIZZLE3(y, z, w);

		SWIZZLE3(y, w, x);
		SWIZZLE3(y, w, y);
		SWIZZLE3(y, w, z);
		SWIZZLE3(y, w, w);

		SWIZZLE3(z, x, x);
		SWIZZLE3(z, x, y);
		SWIZZLE3(z, x, z);
		SWIZZLE3(z, x, w);

		SWIZZLE3(z, y, x);
		SWIZZLE3(z, y, y);
		SWIZZLE3(z, y, z);
		SWIZZLE3(z, y, w);

		SWIZZLE3(z, z, x);
		SWIZZLE3(z, z, y);
		SWIZZLE3(z, z, z);
		SWIZZLE3(z, z, w);

		SWIZZLE3(z, w, x);
		SWIZZLE3(z, w, y);
		SWIZZLE3(z, w, z);
		SWIZZLE3(z, w, w);

		SWIZZLE3(w, x, x);
		SWIZZLE3(w, x, y);
		SWIZZLE3(w, x, z);
		SWIZZLE3(w, x, w);

		SWIZZLE3(w, y, x);
		SWIZZLE3(w, y, y);
		SWIZZLE3(w, y, z);
		SWIZZLE3(w, y, w);

		SWIZZLE3(w, z, x);
		SWIZZLE3(w, z, y);
		SWIZZLE3(w, z, z);
		SWIZZLE3(w, z, w);

		SWIZZLE3(w, w, x);
		SWIZZLE3(w, w, y);
		SWIZZLE3(w, w, z);
		SWIZZLE3(w, w, w);

		SWIZZLE4_FULL(x, y, z, w);
		SWIZZLE4_FULL(y, x, z, w);
		SWIZZLE4_FULL(z, x, y, w);
		SWIZZLE4_FULL(w, x, y, z);
#endif
	};

	// +
	template <class T, size_t n>
	TVector<T, n> operator + (const TVector<T, n>& lhs, T rhs) {
		TVector<T, n> v = lhs;
		for (size_t i = 0; i < n; i++) {
			v[i] += rhs;
		}

		return v;
	}

	template <class T, size_t n>
	TVector<T, n>& operator += (TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] += rhs[i];
		}
		
		return lhs;
	}

	template <class T, size_t n>
	TVector<T, n> operator + (const TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		TVector<T, n> res(lhs);
		res += rhs;
		return res;
	}
	
	template <class T, size_t n>
	TVector<T, n> operator += (TVector<T, n>& lhs, T t) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] += t;
		}
		return lhs;
	}

	// -
	template <class T, size_t n>
	TVector<T, n> operator - (const TVector<T, n>& lhs, T rhs) {
		TVector<T, n> v = lhs;
		for (size_t i = 0; i < n; i++) {
			v[i] -= rhs;
		}

		return v;
	}

	template <class T, size_t n>
	TVector<T, n>& operator -= (TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] -= rhs[i];
		}
		
		return lhs;
	}

	template <class T, size_t n>
	TVector<T, n> operator - (const TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		TVector<T, n> res(lhs);
		res -= rhs;
		return res;
	}
	
	template <class T, size_t n>
	TVector<T, n> operator -= (TVector<T, n>& lhs, T t) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] -= t;
		}
		return lhs;
	}

	// *


	template <class T, size_t n>
	TVector<T, n>& operator *= (TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] *= rhs[i];
		}
		
		return lhs;
	}

	template <class T, size_t n>
	TVector<T, n> operator * (const TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		TVector<T, n> res(lhs);
		res *= rhs;
		return  res;
	}
	
	template <class T, size_t n>
	TVector<T, n> operator * (const TVector<T, n>& lhs, T t) {
		TVector<T, n> result = lhs;
		for (size_t i = 0; i < n; i++) {
			result[i] *= t;
		}
		return result;
	}

	template <class T, size_t n>
	TVector<T, n> operator *= (TVector<T, n>& lhs, T t) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] *= t;
		}
		return lhs;
	}

	// /


	template <class T, size_t n>
	TVector<T, n> operator / (const TVector<T, n>& lhs, T t) {
		TVector<T, n> result = lhs;
		for (size_t i = 0; i < n; i++) {
			result[i] /= t;
		}
		return result;
	}

	template <class T, size_t n>
	TVector<T, n>& operator /= (TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] /= rhs[i];
		}
		
		return lhs;
	}

	template <class T, size_t n>
	TVector<T, n> operator / (const TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		TVector<T, n> res(lhs);
		res /= rhs;
		return res;
	}
	
	template <class T, size_t n>
	TVector<T, n> operator /= (TVector<T, n>& lhs, T t) {
		for (size_t i = 0; i < n; i++) {
			lhs[i] /= t;
		}
		return lhs;
	}

	template <class T, size_t n>
	T DotProduct(const TVector<T, n>& lhs, const TVector<T, n>& rhs) {
		T res(0);
		for (size_t i = 0; i < n; i++) {
			res += lhs[i] * rhs[i];
		}

		return res;
	}

	template <class T>
	T CrossProduct(const TVector<T, 2>& lhs, const TVector<T, 2>& rhs) {
		return lhs[0] * rhs[1] - lhs[1] * rhs[0];
	}

	template <class T>
	TVector<T, 3> CrossProduct(const TVector<T, 3>& lhs, const TVector<T, 3>& rhs) {
		TVector<T, 3> t;
		t[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
		t[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
		t[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];

		return t;
	}

	template <class T>
	bool CrossPoint(TVector<T, 2>& result, const TVector<T, 2>& ps, const TVector<T, 2>& pt, const TVector<T, 2>& qs, const TVector<T, 2>& qt, T& alpha, T& beta) {
		T z = CrossProduct(pt - ps, qt - qs);
		if (fabs(z) < 1e-10)
			return false;

		T A1 = pt[0] - ps[0], A2 = qs[0] - qt[0]; 
		T B1 = pt[1] - ps[1], B2 = qs[1] - qt[1]; 
		T C1 = qs[0] - ps[0];
		T C2 = qs[1] - ps[1];
		T D = A1 * B2 - A2 * B1;

		alpha = (B1 * C2 - B2 * C1) / D;
		beta = (C1 * A2 - C2 * A1) / D;

		result = ((ps + (pt - ps) * alpha) + (qs + (qt - qs) * beta)) / T(2);

		return true;
	}

	template <class T>
	bool Clip(std::pair<T, T>& lhs, const std::pair<T, T>& rhs) {
		bool b = true;
		for (size_t i = 0; i < T::size; i++) {
			lhs.first[i] = Math::Max(lhs.first[i], rhs.first[i]);
			lhs.second[i] = Math::Min(lhs.second[i], rhs.second[i]);

			if (lhs.first[i] > lhs.second[i])
				b = false;
		}

		return b;
	}

	template <class T>
	std::pair<T, T>& Merge(std::pair<T, T>& host, const std::pair<T, T>& rhs) {
		Union(host, rhs.first);
		Union(host, rhs.second);
		return host;
	}

	template <class T>
	std::pair<T, T>& Union(std::pair<T, T>& host, const T& value) {
		for (size_t i = 0; i < T::size; i++) {
			host.first[i] = Math::Min(host.first[i], value[i]);
			host.second[i] = Math::Max(host.second[i], value[i]);
		}

		return host;
	}

	template <class T>
	bool Overlap(const std::pair<T, T>& lhs, const std::pair<T, T>& rhs) {
		for (size_t i = 0; i < T::size; i++) {
			if (rhs.second[i] < lhs.first[i] || lhs.second[i] < rhs.first[i])
				return false;
		}

		return true;
	}

	template <class T>
	bool Contain(const std::pair<T, T>& host, const T& value) {
		for (size_t i = 0; i < T::size; i++) {
			if (value[i] < host.first[i] || host.second[i] < value[i]) {
				return false;
			}
		}

		return true;
	}

	template <class T>
	bool Contain(const std::pair<T, T>& host, const std::pair<T, T>& value) {
		return Contain(host, value.first) && Contain(host, value.second);
	}

	template <class T>
	T ToLocal(const std::pair<T, T>& val, const T& t) {
		T r;
		for (size_t i = 0; i < T::size; i++) {
			r[i] = (t[i] - val.first[i]) / (val.second[i] - val.first[i]);
		}

		return r;
	}

	template <class T>
	T FromLocal(const std::pair<T, T>& val, const T& t) {
		T r;
		for (size_t i = 0; i < T::size; i++) {
			r[i] = (val.second[i] - val.first[i]) * t[i] + val.first[i];
		}

		return r;
	}

	template <class T, class D>
	std::pair<T, D> operator * (const std::pair<T, D>& lhs, double t) {
		return std::pair<T, D>(lhs.first * t, lhs.second * t);
	}

	template <class T, class D>
	std::pair<T, D> operator / (const std::pair<T, D>& lhs, double t) {
		return std::pair<T, D>(lhs.first / t, lhs.second / t);
	}
}


#endif // __TVECTOR_H__
