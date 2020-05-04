// TMatrix.h -- Basic operations on matrix
// By PaintDream (paintdream@paintdream.com)
// 2014-12-1
//

#ifndef __TMATRIX_H__
#define __TMATRIX_H__


#include "../PaintsNow.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include "TAlgorithm.h"
#include "TVector.h"

namespace PaintsNow {
	template <class T, size_t m = 4, size_t n = m>
	class TMatrix {
	public:
		enum { M = m, N = n };
		typedef T type;

		TMatrix(const T* value) {
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					data[i][j] = *value++;
				}
			}
		}

		template <class D, size_t s, size_t t>
		explicit TMatrix(const TMatrix<D, s, t>& mat) {
			*this = TMatrix<T, m, n>();

			for (size_t i = 0; i < Min(m, s); i++) {
				for (size_t j = 0; j < Min(n, t); j++) {
					data[i][j] = mat.data[i][j];
				}
			}
		}

		TMatrix() {
			size_t z = Min(m, n);
			memset(data, 0, sizeof(data));
			for (size_t i = 0; i < z; i++) {
				data[i][i] = 1;
			}
		}

		TMatrix<T, n, m> Transpose() const {
			TMatrix<T, n, m> target;
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					target(j, i) = (*this)(i, j);
				}
			}

			return target;
		}

		static T Distance(const TMatrix<T, m, n>& lhs, const TMatrix<T, m, n>& rhs) {
			TMatrix<T, m, n> diff;
			TMatrix<T, n, m> star;
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					star(j, i) = diff(i, j) = lhs(i, j) - rhs(i, j);
				}
			}

			TMatrix<T, n, n> result = star * diff;
			T length = 0;
			for (size_t k = 0; k < n; k++) {
				length += result(k, k);
			}

			return (T)sqrt(length);
		}

		operator T* () {
			return data;
		}

		bool operator == (const TMatrix& rhs) const {
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					if (data[i][j] != rhs.data[i][j])
						return false;
				}
			}

			return true;
		}

		bool operator != (const TMatrix& rhs) const {
			return !(*this == rhs);
		}

		T& operator () (size_t i, size_t j) {
			return data[i][j];
		}

		const T& operator () (size_t i, size_t j) const {
			return data[i][j];
		}

		T data[m][n]; // m: col index, n: row index
	};

	template <class T, size_t m, size_t n>
	TMatrix<T, m, n> Scale(const TMatrix<T, m, n>& lhs, const TVector<T, m>& v) {
		TMatrix<T, m, n> mat;
		for (size_t i = 0; i < Min(m, n); i++) {
			mat.data[i][i] = v[i];
		}

		return mat * lhs;
	}


#if defined(_MSC_VER) && _MSC_VER <= 1200
	template <class T, size_t m, size_t n>
	TMatrix<T, m, m> operator * (const TMatrix<T, m, n>& lhs, const TMatrix<T, n, m>& rhs) {
		TMatrix<T, m, m> ret;
		for (size_t i = 0; i < m; i++) {
			for (size_t j = 0; j < m; j++) {
				T sum(0);
				for (size_t k = 0; k < n; k++) {
					sum += lhs(i, k) * rhs(k, j);
				}

				ret(i, j) = sum;
			}
		}

		return ret;
	}
#else
	template <class T, size_t m, size_t n, size_t p>
	TMatrix<T, m, p> operator * (const TMatrix<T, m, n>& lhs, const TMatrix<T, n, p>& rhs) {
		TMatrix<T, m, p> ret;
		for (size_t i = 0; i < m; i++) {
			for (size_t j = 0; j < p; j++) {
				T sum(0);
				for (size_t k = 0; k < n; k++) {
					sum += lhs(i, k) * rhs(k, j);
				}

				ret(i, j) = sum;
			}
		}

		return ret;
	}
#endif

	template <class T, size_t m, size_t n>
	TVector<T, n> operator * (const TVector<T, m>& value, const TMatrix<T, m, n>& rhs) {
		TVector<T, n> ret;
		for (size_t i = 0; i < n; i++) {
			ret[i] = 0;
			for (size_t j = 0; j < m; j++) {
				ret[i] += value[j] * rhs(j, i);
			}
		}

		return ret;
	}

	// from: https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/matrices.cpp
	template <class T, size_t n, size_t m>
	TMatrix<T, m, n> QuickInverse(const TMatrix<T, m, n>& mat) {
		static_assert(m == n && m == 4, "QuickInverse only applies to 4x4 matrix");
		T x = TType3<T>(mat(0, 0), mat(0, 1), mat(0, 2)).Length();
		T y = TType3<T>(mat(1, 0), mat(1, 1), mat(1, 2)).Length();
		T z = TType3<T>(mat(2, 0), mat(2, 1), mat(2, 2)).Length();

		T xx = x * x;
		T yy = y * y;
		T zz = z * z;
		T xy = x * y;
		T xz = x * z;
		T yz = y * z;

		TMatrix<T, n, n> inverse;
		inverse(0, 0) = mat(0, 0) / xx;
		inverse(0, 1) = mat(1, 0) / xy;
		inverse(0, 2) = mat(2, 0) / xz;
		inverse(1, 0) = mat(0, 1) / xy;
		inverse(1, 1) = mat(1, 1) / yy;
		inverse(1, 2) = mat(2, 1) / yz;
		inverse(2, 0) = mat(0, 2) / xy;
		inverse(2, 1) = mat(1, 2) / yy;
		inverse(2, 2) = mat(2, 2) / yz;

		TType3<T> right = TType3<T>(inverse(0, 0), inverse(1, 0), inverse(2, 0));
		TType3<T> up = TType3<T>(inverse(0, 1), inverse(1, 1), inverse(2, 1));
		TType3<T> forward = TType3<T>(inverse(0, 2), inverse(1, 2), inverse(2, 2));
		TType3<T> position = TType3<T>(mat(3, 0), mat(3, 1), mat(3, 2));

		inverse(3, 0) = -DotProduct(right, position);
		inverse(3, 1) = -DotProduct(up, position);
		inverse(3, 2) = -DotProduct(forward, position);

		return inverse;
	}

	template <class T, size_t n, size_t k>
	TMatrix<T, k, n> Inverse(const TMatrix<T, k, n>& m) {
		static_assert(k == n && k == 4, "Inverse only applies to 4x4 matrix");
		T det
			= m(0, 0) * m(1, 1) * m(2, 2) * m(3, 3) + m(0, 0) * m(1, 2) * m(2, 3) * m(3, 1) + m(0, 0) * m(1, 3) * m(2, 1) * m(3, 2)
			+ m(0, 1) * m(1, 0) * m(2, 3) * m(3, 2) + m(0, 1) * m(1, 2) * m(2, 0) * m(3, 3) + m(0, 1) * m(1, 3) * m(2, 2) * m(3, 0)
			+ m(0, 2) * m(1, 0) * m(2, 1) * m(3, 3) + m(0, 2) * m(1, 1) * m(2, 3) * m(3, 0) + m(0, 2) * m(1, 3) * m(2, 0) * m(3, 1)
			+ m(0, 3) * m(1, 0) * m(2, 2) * m(3, 1) + m(0, 3) * m(1, 1) * m(2, 0) * m(3, 2) + m(0, 3) * m(1, 2) * m(2, 1) * m(3, 0)
			- m(0, 0) * m(1, 1) * m(2, 3) * m(3, 2) - m(0, 0) * m(1, 2) * m(2, 1) * m(3, 3) - m(0, 0) * m(1, 3) * m(2, 2) * m(3, 1)
			- m(0, 1) * m(1, 0) * m(2, 2) * m(3, 3) - m(0, 1) * m(1, 2) * m(2, 3) * m(3, 0) - m(0, 1) * m(1, 3) * m(2, 0) * m(3, 2)
			- m(0, 2) * m(1, 0) * m(2, 3) * m(3, 1) - m(0, 2) * m(1, 1) * m(2, 0) * m(3, 3) - m(0, 2) * m(1, 3) * m(2, 1) * m(3, 0)
			- m(0, 3) * m(1, 0) * m(2, 1) * m(3, 2) - m(0, 3) * m(1, 1) * m(2, 2) * m(3, 0) - m(0, 3) * m(1, 2) * m(2, 0) * m(3, 1);

		if (fabs(det) < 0.000001f) {
			return TMatrix<T, k, n>();
		}

		T idet = (T)(1.0 / det);
		TMatrix<T, k, n> result;
		result(0, 0) = (m(1, 1) * m(2, 2) * m(3, 3) + m(1, 2) * m(2, 3) * m(3, 1) + m(1, 3) * m(2, 1) * m(3, 2) - m(1, 1) * m(2, 3) * m(3, 2) - m(1, 2) * m(2, 1) * m(3, 3) - m(1, 3) * m(2, 2) * m(3, 1)) * idet;
		result(0, 1) = (m(0, 1) * m(2, 3) * m(3, 2) + m(0, 2) * m(2, 1) * m(3, 3) + m(0, 3) * m(2, 2) * m(3, 1) - m(0, 1) * m(2, 2) * m(3, 3) - m(0, 2) * m(2, 3) * m(3, 1) - m(0, 3) * m(2, 1) * m(3, 2)) * idet;
		result(0, 2) = (m(0, 1) * m(1, 2) * m(3, 3) + m(0, 2) * m(1, 3) * m(3, 1) + m(0, 3) * m(1, 1) * m(3, 2) - m(0, 1) * m(1, 3) * m(3, 2) - m(0, 2) * m(1, 1) * m(3, 3) - m(0, 3) * m(1, 2) * m(3, 1)) * idet;
		result(0, 3) = (m(0, 1) * m(1, 3) * m(2, 2) + m(0, 2) * m(1, 1) * m(2, 3) + m(0, 3) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 2) * m(2, 3) - m(0, 2) * m(1, 3) * m(2, 1) - m(0, 3) * m(1, 1) * m(2, 2)) * idet;
		result(1, 0) = (m(1, 0) * m(2, 3) * m(3, 2) + m(1, 2) * m(2, 0) * m(3, 3) + m(1, 3) * m(2, 2) * m(3, 0) - m(1, 0) * m(2, 2) * m(3, 3) - m(1, 2) * m(2, 3) * m(3, 0) - m(1, 3) * m(2, 0) * m(3, 2)) * idet;
		result(1, 1) = (m(0, 0) * m(2, 2) * m(3, 3) + m(0, 2) * m(2, 3) * m(3, 0) + m(0, 3) * m(2, 0) * m(3, 2) - m(0, 0) * m(2, 3) * m(3, 2) - m(0, 2) * m(2, 0) * m(3, 3) - m(0, 3) * m(2, 2) * m(3, 0)) * idet;
		result(1, 2) = (m(0, 0) * m(1, 3) * m(3, 2) + m(0, 2) * m(1, 0) * m(3, 3) + m(0, 3) * m(1, 2) * m(3, 0) - m(0, 0) * m(1, 2) * m(3, 3) - m(0, 2) * m(1, 3) * m(3, 0) - m(0, 3) * m(1, 0) * m(3, 2)) * idet;
		result(1, 3) = (m(0, 0) * m(1, 2) * m(2, 3) + m(0, 2) * m(1, 3) * m(2, 0) + m(0, 3) * m(1, 0) * m(2, 2) - m(0, 0) * m(1, 3) * m(2, 2) - m(0, 2) * m(1, 0) * m(2, 3) - m(0, 3) * m(1, 2) * m(2, 0)) * idet;
		result(2, 0) = (m(1, 0) * m(2, 1) * m(3, 3) + m(1, 1) * m(2, 3) * m(3, 0) + m(1, 3) * m(2, 0) * m(3, 1) - m(1, 0) * m(2, 3) * m(3, 1) - m(1, 1) * m(2, 0) * m(3, 3) - m(1, 3) * m(2, 1) * m(3, 0)) * idet;
		result(2, 1) = (m(0, 0) * m(2, 3) * m(3, 1) + m(0, 1) * m(2, 0) * m(3, 3) + m(0, 3) * m(2, 1) * m(3, 0) - m(0, 0) * m(2, 1) * m(3, 3) - m(0, 1) * m(2, 3) * m(3, 0) - m(0, 3) * m(2, 0) * m(3, 1)) * idet;
		result(2, 2) = (m(0, 0) * m(1, 1) * m(3, 3) + m(0, 1) * m(1, 3) * m(3, 0) + m(0, 3) * m(1, 0) * m(3, 1) - m(0, 0) * m(1, 3) * m(3, 1) - m(0, 1) * m(1, 0) * m(3, 3) - m(0, 3) * m(1, 1) * m(3, 0)) * idet;
		result(2, 3) = (m(0, 0) * m(1, 3) * m(2, 1) + m(0, 1) * m(1, 0) * m(2, 3) + m(0, 3) * m(1, 1) * m(2, 0) - m(0, 0) * m(1, 1) * m(2, 3) - m(0, 1) * m(1, 3) * m(2, 0) - m(0, 3) * m(1, 0) * m(2, 1)) * idet;
		result(3, 0) = (m(1, 0) * m(2, 2) * m(3, 1) + m(1, 1) * m(2, 0) * m(3, 2) + m(1, 2) * m(2, 1) * m(3, 0) - m(1, 0) * m(2, 1) * m(3, 2) - m(1, 1) * m(2, 2) * m(3, 0) - m(1, 2) * m(2, 0) * m(3, 1)) * idet;
		result(3, 1) = (m(0, 0) * m(2, 1) * m(3, 2) + m(0, 1) * m(2, 2) * m(3, 0) + m(0, 2) * m(2, 0) * m(3, 1) - m(0, 0) * m(2, 2) * m(3, 1) - m(0, 1) * m(2, 0) * m(3, 2) - m(0, 2) * m(2, 1) * m(3, 0)) * idet;
		result(3, 2) = (m(0, 0) * m(1, 2) * m(3, 1) + m(0, 1) * m(1, 0) * m(3, 2) + m(0, 2) * m(1, 1) * m(3, 0) - m(0, 0) * m(1, 1) * m(3, 2) - m(0, 1) * m(1, 2) * m(3, 0) - m(0, 2) * m(1, 0) * m(3, 1)) * idet;
		result(3, 3) = (m(0, 0) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 2) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1) - m(0, 0) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 0) * m(2, 2) - m(0, 2) * m(1, 1) * m(2, 0)) * idet;

		return result;
	}
}


#endif // __TMATRIX_H__
