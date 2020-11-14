#include "IType.h"
#include <cmath>
#include <iostream>
#include <cfloat>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

#ifdef MATH_SSE
#include <xmmintrin.h>
#endif

#ifdef UNIFY_STRING_LAYOUT
const size_t PaintsNow::String::npos = -1;
#endif
namespace PaintsNow {
	std::vector<String> Split(const String& str, char sep) {
		std::vector<String> result;
		String current;
		for (size_t i = 0; i < str.length(); i++) {
			char ch = str[i];
			if (ch == sep) {
				if (!current.empty()) {
					result.emplace_back(current);
					current = "";
				}
			} else {
				current += ch;
			}
		}

		if (!current.empty())
			result.emplace_back(std::move(current));

		return result;
	}

	uint32_t HashBuffer(const void* buffer, size_t length) {
		unsigned char *key = (unsigned char *)buffer;
		unsigned char *end = key + length;
		uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
		int ch;
		while (key != end) {
			ch = *key++;
			seed1 = ch ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}

		return seed1;
	}

	namespace Math {
		MatrixFloat4x4 Rotate3D(const MatrixFloat4x4& input, float rad, const Float3& d) {
			const TVector<float, 3>& dir = d;
			const double PI = 3.1415926;
			float c = (float)cos(rad);
			float s = (float)sin(rad);

			float mat[16] = {
				c + (1 - c) * dir[0] * dir[0], (1 - c) * dir[1] * dir[0] + s * dir[2], (1 - c) * dir[2] * dir[0] - s * dir[1], 0,
				(1 - c) * dir[0] * dir[1] - s * dir[2], c + (1 - c) * dir[1] * dir[1], (1 - c) * dir[2] * dir[1] + s * dir[0], 0,
				(1 - c) * dir[0] * dir[2] + s * dir[1], (1 - c) * dir[1] * dir[2] - s * dir[0], c + (1 - c) * dir[2] * dir[2], 0,
				0, 0, 0, 1
			};

			return MatrixFloat4x4(mat) * input;
		}

		MatrixFloat4x4 Ortho(const Float3& size) {
			float f[16] = { 1.0f / size.x(), 0, 0, 0, 0, 1.0f / size.y(), 0, 0, 0, 0, 1.0f / size.z(), 0, 0, 0, 0.0f, 1.0f };
			return MatrixFloat4x4(f);
		}

		MatrixFloat4x4 Perspective(float d, float rr, float n, float f) {
			float t = n * (float)tan(d / 2.0f);
			float r = rr * t;
			float trans[16] = {
				n / r, 0, 0, 0,
				0, n / t, 0, 0,
				0, 0, (f + n) / (f - n), -1,
				0, 0, 2 * f * n / (f - n), 0
			};

			return MatrixFloat4x4(trans);
		}

		MatrixFloat4x4 InverseProjection(const MatrixFloat4x4& m) {
			float a = m(0, 0);
			float b = m(1, 1);
			float c = m(2, 2);
			float d = m(3, 2);
			float s = m(2, 0);
			float t = m(2, 1);

			float mat[16] = {
				1 / a, 0, 0, 0,
				0, 1 / b, 0, 0,
				0, 0, 0, 1 / d,
				-s / a, -t / b, -1, c / d
			};

			return mat;
		}

		MatrixFloat4x4 LookAt(const Float3& position, const Float3& dir, const Float3& u) {
			Float3 direction = dir;
			Float3 up = u;

			direction.Normalize();
			up.Normalize();

			/* Side = forward x up */
			Float3 side = CrossProduct(direction, up);
			side.Normalize();
			up = CrossProduct(side, direction);

			MatrixFloat4x4 m;

			m(0, 0) = side[0];
			m(1, 0) = side[1];
			m(2, 0) = side[2];
			m(3, 0) = 0;

			m(0, 1) = up[0];
			m(1, 1) = up[1];
			m(2, 1) = up[2];
			m(3, 1) = 0;

			m(0, 2) = -direction[0];
			m(1, 2) = -direction[1];
			m(2, 2) = -direction[2];
			m(3, 2) = 0;

			m(0, 3) = 0;
			m(1, 3) = 0;
			m(2, 3) = 0;
			m(3, 3) = 1;

			return Translate3D(m, -position);
		}

		MatrixFloat4x4 Translate3D(const MatrixFloat4x4& input, const Float3& v) {
			float trans[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, v.x(), v.y(), v.z(), 1 };
			return MatrixFloat4x4(trans) * input;
		}

		Float3 Transform3D(const MatrixFloat4x4& input, const Float3& v) {
			Float4 position(v.x(), v.y(), v.z(), 1.0f);
			position = position * input;
			return Float3(position.x() / position.w(), position.y() / position.w(), position.z() / position.w());
		}

		bool Capture3D(const MatrixFloat4x4& matrix, const Float3Pair& vec, const Float2Pair& size) {
			Float2Pair rect;
			float dist = FLT_MAX;
			float maxDist = -FLT_MAX;

			for (int i = 0; i < 8; i++) {
				Float4 v = Float4((i & 1) ? vec.second.x() : vec.first.x(), (i & 2) ? vec.second.y() : vec.first.y(), (i & 4) ? vec.second.z() : vec.first.z(), 1) * matrix;
				dist = dist < v.z() ? dist : v.z();
				maxDist = maxDist > v.z() ? maxDist : v.z();
				Float2 p(v.x() / v.w(), v.y() / v.w());
				if (i == 0) rect = Float2Pair(p, p);
				else Union(rect, p);
			}

			return maxDist >= 0 && Overlap(rect, size);
		}

		template <class T>
		bool FloatEqual(T a, T b) {
			return fabs(a - b) < 1e-6;
		}

		template <class T, size_t n>
		class LinearEquation {
		public:
			static int TotalChoiceGauss(TMatrix<T, n, n>& a, T b[]) {
				T MaxValue, tmp;
				int l(1), i, j, is;
				bool yn;

				int js[n];
				int k;

				for (k = 0; k < n - 1; k++) {
					MaxValue = 0.0;
					js[k] = 0;

					for (i = k; i < n; i++)
						for (j = k; j < n; j++) {
							tmp = fabs(a(i, j));
							if (tmp > MaxValue) {
								MaxValue = tmp;
								js[k] = j;
								is = i;
							}
						}

					yn = FloatEqual(MaxValue, (T)0);
					if (yn) l = 0;
					else {
						if (js[k] != k)
							for (i = 0; i < n; i++) std::swap(a(i, k), a(i, js[k]));

						if (is != k) {
							for (j = k; j < n; j++)	std::swap(a(k, j), a(is, j));

							std::swap(b[k], b[is]);
						}
					}

					if (l == 0) {
						return 0;
					}

					MaxValue = fabs(a(k, k));

					for (j = k + 1; j < n; j++)	a(k, j) /= a(k, k);

					b[k] /= a(k, k);
					for (i = k + 1; i < n; i++) {
						for (j = k + 1; j < n; j++) {
							a(i, j) = a(i, j) - a(i, k) * a(k, j);
						}

						b[i] = b[i] - a(i, k) * b[k];
					}
				}

				MaxValue = fabs(a((n - 1), (n - 1)));

				yn = FloatEqual(MaxValue, (T)0);
				if (yn) {
					return(0);
				}

				b[n - 1] /= a((n - 1), (n - 1));

				for (i = n - 2; i >= 0; i--) {
					T t = 0.0;

					for (j = i + 1; j < n; j++)	t = t + a(i, j) * b[j];

					b[i] = b[i] - t;
				}

				js[n - 1] = n - 1;
				for (k = n - 2; k >= 0; k--)
					if (js[k] != k)
						std::swap(b[k], b[js[k]]);

				return (1);
			}
		};

		bool Intersect3D(Float3& res, Float2& uv, const Float3 face[3], const Float3Pair& vec) {
			// handle size!!!!
			const Float3& v = vec.second;
			const Float3& u = vec.first;
			const Float3& base = face[0];
			const Float3& m = face[1];
			const Float3& n = face[2];

			Float3 N = n - base;
			Float3 M = m - base;
			// make linear equations
			MatrixFloat3x3 mat;
			mat(0, 0) = M.x(); mat(0, 1) = N.x(); mat(0, 2) = -v.x();
			mat(1, 0) = M.y(); mat(1, 1) = N.y(); mat(1, 2) = -v.y();
			mat(2, 0) = M.z(); mat(2, 1) = N.z(); mat(2, 2) = -v.z();
			float target[3];
			target[0] = u.x() - base.x(); target[1] = u.y() - base.y(); target[2] = u.z() - base.z();

			if (!LinearEquation<float, 3>::TotalChoiceGauss(mat, target)) {
				return false;
			}

			float alpha = target[0];
			float beta = target[1];

			res = base + M * alpha + N * beta;

			const double EPSILON = 1e-4;
			uv = Float2(alpha, beta);
			return (alpha >= -EPSILON && beta >= -EPSILON && (alpha + beta <= 1 + EPSILON));
		}

		// https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define fminf Math::Min
#define fmaxf Math::Max
#endif

		bool IntersectBox(const Float3Pair& aabb, const Float3Pair& ray) {
			const Float3& minValue = aabb.first;
			const Float3& maxValue = aabb.second;

			float dx = ray.second.x();
			float dy = ray.second.y();
			float dz = ray.second.z();
			float dxdy = dx * dy;
			float dydz = dy * dz;
			float dxdz = dx * dz;
			float sign = dxdy * dz >= 0;

			float t1 = (minValue.x() - ray.first.x()) * (dydz * sign);
			float t2 = (maxValue.x() - ray.first.x()) * (dydz * sign);
			float t3 = (minValue.y() - ray.first.y()) * (dxdz * sign);
			float t4 = (maxValue.y() - ray.first.y()) * (dxdz * sign);
			float t5 = (minValue.z() - ray.first.z()) * (dxdy * sign);
			float t6 = (maxValue.z() - ray.first.z()) * (dxdy * sign);

			float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
			float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

			return tmax >= 0 && tmin <= tmax;
		}
	}

#ifdef _WIN32
	String Utf8ToSystem(const String& str) {
		DWORD dwMinSize;
		dwMinSize = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		String ret;
		ret.resize((size_t)dwMinSize * sizeof(WCHAR) + sizeof(WCHAR) * 2, 0);
		::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), (WCHAR*)(ret.data()), dwMinSize);
		return ret;
	}

	String SystemToUtf8(const String& str) {
		DWORD dwMinSize;
		dwMinSize = ::WideCharToMultiByte(CP_UTF8, 0, (const WCHAR*)str.data(), (int)str.size() / sizeof(WCHAR), nullptr, 0, nullptr, nullptr);
		String ret;
		ret.resize((size_t)dwMinSize, 0);
		::WideCharToMultiByte(CP_UTF8, 0, (const WCHAR*)str.data(), (int)str.size() / sizeof(WCHAR), (char*)(ret.data()), dwMinSize, nullptr, nullptr);
		return ret;
	}
#endif

}
