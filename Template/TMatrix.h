// TMatrix.h -- Basic operations on matrix
// By PaintDream (paintdream@paintdream.com)
// 2014-12-1
//

#pragma once

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

		forceinline TMatrix(const T* value) {
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					data[i][j] = *value++;
				}
			}
		}

		template <class D, size_t s, size_t t>
		forceinline explicit TMatrix(const TMatrix<D, s, t>& mat) {
			*this = TMatrix<T, m, n>();

			for (size_t i = 0; i < Math::Min(m, s); i++) {
				for (size_t j = 0; j < Math::Min(n, t); j++) {
					data[i][j] = mat.data[i][j];
				}
			}
		}

		static const TMatrix CreateIdentity() {
			TMatrix mat;
			T (*data)[n] = mat.data;
			size_t z = Math::Min(m, n);
			memset(data, 0, sizeof(mat.data));
			for (size_t i = 0; i < z; i++) {
				data[i][i] = 1;
			}

			return mat;
		}

		static const TMatrix& Identity() {
			static TMatrix matrix = CreateIdentity();
			return matrix;
		}

		forceinline TMatrix() {}

		forceinline TMatrix<T, n, m> Transpose() const {
			TMatrix<T, n, m> target;
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					target(j, i) = (*this)(i, j);
				}
			}

			return target;
		}

		forceinline static T Distance(const TMatrix<T, m, n>& lhs, const TMatrix<T, m, n>& rhs) {
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

		forceinline operator T* () {
			return data;
		}

		forceinline bool operator == (const TMatrix& rhs) const {
			for (size_t i = 0; i < m; i++) {
				for (size_t j = 0; j < n; j++) {
					if (data[i][j] != rhs.data[i][j])
						return false;
				}
			}

			return true;
		}

		forceinline bool operator != (const TMatrix& rhs) const {
			return !(*this == rhs);
		}

		forceinline TVector<T, n>& operator () (size_t i) {
			return *reinterpret_cast<TVector<T, n>*>(&data[i][0]);
		}

		forceinline const TVector<T, n>& operator () (size_t i) const {
			return *reinterpret_cast<const TVector<T, n>*>(&data[i][0]);
		}

		forceinline T& operator () (size_t i, size_t j) {
			return data[i][j];
		}

		forceinline const T& operator () (size_t i, size_t j) const {
			return data[i][j];
		}

		T data[m][n]; // m: col index, n: row index
	};


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
		TMatrix<T, n, p> trans = rhs.Transpose();
		TMatrix<T, m, p> ret;
		for (size_t i = 0; i < m; i++) {
			const TVector<T, p>& left = lhs(i);
			for (size_t j = 0; j < p; j++) {
				ret(i, j) = DotProduct(left, trans(j));
			}
		}

		return ret;
	}
#endif

	template <class T, size_t m, size_t n>
	TVector<T, n> operator * (const TVector<T, m>& value, const TMatrix<T, m, n>& rhs) {
		TMatrix<T, n, m> trans = rhs.Transpose();
		TVector<T, n> ret;
		for (size_t i = 0; i < n; i++) {
			ret[i] = DotProduct(value, trans(i));
		}

		return ret;
	}

	namespace Math {
		template <class T, size_t m, size_t n>
		forceinline TMatrix<T, m, n> Scale(const TMatrix<T, m, n>& lhs, const TVector<T, m>& v) {
			TMatrix<T, m, n> mat = TMatrix<T, m, n>::Identity();
			for (size_t i = 0; i < Math::Min(m, n); i++) {
				mat.data[i][i] = v[i];
			}

			return mat * lhs;
		}

		// from: https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/matrices.cpp
		template <class T, size_t n, size_t m>
		TMatrix<T, m, n> QuickInverse(const TMatrix<T, m, n>& mat) {
			static_assert(m == n && m == 4, "QuickInverse only applies to 4x4 matrix");
			TType4<T> vv;
			vv.x() = T(1) / TType4<T>(mat(0, 0), mat(0, 1), mat(0, 2), T(0)).SquareLength();
			vv.y() = T(1) / TType4<T>(mat(1, 0), mat(1, 1), mat(1, 2), T(0)).SquareLength();
			vv.z() = T(1) / TType4<T>(mat(2, 0), mat(2, 1), mat(2, 2), T(0)).SquareLength();
			vv.w() = 0;

			TMatrix<T, n, n> inverse = mat.Transpose();
			for (size_t i = 0; i < 3; i++) {
				inverse(i) = inverse(i) * vv;
			}

			TType4<T> right = TType4<T>(inverse(0, 0), inverse(1, 0), inverse(2, 0), T(0));
			TType4<T> up = TType4<T>(inverse(0, 1), inverse(1, 1), inverse(2, 1), T(0));
			TType4<T> forward = TType4<T>(inverse(0, 2), inverse(1, 2), inverse(2, 2), T(0));
			const TVector<T, 4>& position = mat(3);

			inverse(3, 0) = -DotProduct(right, position);
			inverse(3, 1) = -DotProduct(up, position);
			inverse(3, 2) = -DotProduct(forward, position);
			inverse(3, 3) = 1;

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
}
