// ZType.h -- Basic type instances
// By PaintDream (paintdream@paintdream.com)
// 2014-11-30
//

#pragma once
#ifdef _MSC_VER
#pragma warning (disable:4786)
#pragma warning (disable:4503)
#endif // _MSC_VER

#include "../PaintsNow.h"
#include "../Template/TVector.h"
#include "../Template/TMatrix.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <cfloat>

#ifndef __STD_TYPES__
#define __STD_TYPES__
#ifdef _MSC_VER
#if (_MSC_VER <= 1200)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef __int64 int64_t;
#else
#include <stdint.h>
#endif
#else
#include <stdint.h>
// typedef unsigned long long uint64_t;
#endif
#endif

namespace PaintsNow {
#if defined(_MSC_VER) && _MSC_VER <= 1200
	class String : public std::string {
	public:
		String() {}
		typedef std::string Base;
		String(const char* s) : Base(s) {}
		String(const char* s, size_t length) : Base(s, length) {}
		String(const std::string& s) : Base(s) {}
		String(const String& s, size_t pos, size_t len = std::string::npos) : Base(s, pos, len) {}
		String(size_t n, char c) : Base(n, c) {}
		template <class It>
		String(It begin, It end) : Base(begin, end) {}

		explicit String(rvalue<String>& s) { std::swap(*this, *s.pointer); }
		String& operator = (const char* s) {
			this->Base::operator = (s);
			return *this;
		}

		String& operator = (const std::string& s) {
			this->Base::operator = (s);
			return *this;
		}

		String& operator = (rvalue<String> s) {
			std::swap(*this, *s.pointer);
			return *this;
		}
	};
#else
	typedef std::string String;
#endif
	uint32_t HashBuffer(const void* buffer, size_t length);
	std::vector<String> Split(const String& str, char sep = ' ');

	typedef TType2<char> Char2;
	typedef TType3<char> Char3;
	typedef TType4<char> Char4;
	typedef TType2<unsigned char> UChar2;
	typedef TType3<unsigned char> UChar3;
	typedef TType4<unsigned char> UChar4;
	typedef TType2<short> Short2;
	typedef TType3<short> Short3;
	typedef TType4<short> Short4;
	typedef TType2<unsigned short> UShort2;
	typedef TType3<unsigned short> UShort3;
	typedef TType4<unsigned short> UShort4;
	typedef TType2<int> Int2;
	typedef TType3<int> Int3;
	typedef TType4<int> Int4;
	typedef TType2<unsigned int> UInt2;
	typedef TType3<unsigned int> UInt3;
	typedef TType4<unsigned int> UInt4;
	typedef TType2<float> Float2;
	typedef TType3<float> Float3;
	typedef TType4<float> Float4;
	typedef TType2<double> Double2;
	typedef TType3<double> Double3;
	typedef TType4<double> Double4;
	typedef std::pair<Float4, Float4> Float4Pair;
	typedef std::pair<Float3, Float3> Float3Pair;
	typedef std::pair<Float2, Float2> Float2Pair;
	typedef std::pair<Double4, Double4> Double4Pair;
	typedef std::pair<Double3, Double3> Double3Pair;
	typedef std::pair<Double2, Double2> Double2Pair;
	typedef std::pair<Int4, Int4> Int4Pair;
	typedef std::pair<Int3, Int3> Int3Pair;
	typedef std::pair<Int2, Int2> Int2Pair;
	typedef std::pair<UInt4, UInt4> UInt4Pair;
	typedef std::pair<UInt3, UInt3> UInt3Pair;
	typedef std::pair<UInt2, UInt2> UInt2Pair;
	typedef std::pair<Short4, Short4> Short4Pair;
	typedef std::pair<Short3, Short3> Short3Pair;
	typedef std::pair<Short2, Short2> Short2Pair;
	typedef std::pair<UShort4, UShort4> UShort4Pair;
	typedef std::pair<UShort3, UShort3> UShort3Pair;
	typedef std::pair<UShort2, UShort2> UShort2Pair;
	typedef TMatrix<float, 4U, 4U> MatrixFloat4x4;
	typedef TMatrix<float, 3U, 4U> MatrixFloat3x4;
	typedef TMatrix<float, 3U, 3U> MatrixFloat3x3;
	typedef TMatrix<int, 3U, 3U> MatrixInt3x3;

#ifdef _WIN32
	String Utf8ToSystem(const String& str);
	String SystemToUtf8(const String& str);
#else
#define Utf8ToSystem(f) (f)
#define SystemToUtf8(f) (f)
#endif

	namespace Math {
		MatrixFloat4x4 Rotate3D(const MatrixFloat4x4& input, float degree, const Float3& d);
		MatrixFloat4x4 Translate3D(const MatrixFloat4x4& input, const Float3& v);
		Float3 Transform3D(const MatrixFloat4x4& input, const Float3& v);
		MatrixFloat4x4 LookAt(const Float3& position, const Float3& dir, const Float3& u);
		MatrixFloat4x4 Ortho(const Float3& size);
		MatrixFloat4x4 Perspective(float d, float rr, float n, float f);
		MatrixFloat4x4 InverseProjectionMatrix(const MatrixFloat4x4& m);

		bool Capture3D(const MatrixFloat4x4& matrix, const Float3Pair& vec, const Float2Pair& size);
		bool Intersect3D(Float3& res, Float2& uv, const Float3 face[3], const Float3Pair& line);
		bool IntersectBox(const Float3Pair& aabb, const Float3Pair& ray);

		// From: https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
		inline uint32_t Log2(uint32_t value) {
			assert(value != 0);
#if defined(_MSC_VER)
			unsigned long index;
#if _MSC_VER <= 1200
			_asm {
				BSF EAX, value;
				MOV index, EAX;
			}
#else
			_BitScanForward(&index, value);
#endif
			
			return index;
#else
			return __builtin_ctzl(value);
#endif
			/*
			static const uint32_t tab32[32] = {
				0,  9,  1, 10, 13, 21,  2, 29,
				11, 14, 16, 18, 22, 25,  3, 30,
				8, 12, 20, 28, 15, 17, 24,  7,
				19, 27, 23,  6, 26,  5,  4, 31 };
			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;
			return tab32[(uint32_t)(value * 0x07C4ACDD) >> 27];*/
		}

		inline uint32_t Log2(uint64_t value) {
			assert(value != 0);
#if defined(_MSC_VER)
#if !_WIN64
			/*
			const uint32_t tab64[64] = {
				63,  0, 58,  1, 59, 47, 53,  2,
				60, 39, 48, 27, 54, 33, 42,  3,
				61, 51, 37, 40, 49, 18, 28, 20,
				55, 30, 34, 11, 43, 14, 22,  4,
				62, 57, 46, 52, 38, 26, 32, 41,
				50, 36, 17, 19, 29, 10, 13, 21,
				56, 45, 25, 31, 35, 16,  9, 12,
				44, 24, 15,  8, 23,  7,  6,  5 };

			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;
			value |= value >> 32;
			return tab64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
			*/
			uint32_t lowPart = (uint32_t)(value & 0xFFFFFFFF);
			return lowPart == 0 ? Log2((uint32_t)((value >> 31) >> 1)) + 32 : Log2(lowPart);
#else
			unsigned long index;
			_BitScanForward64(&index, value);
			return index;
#endif
#else
			return __builtin_ctzll(value);
#endif
		}
	}

	template <class T>
	class Quaternion : public TType4<T> {
	public:
		Quaternion(T ww = 1, T xx = 0, T yy = 0, T zz = 0) : Float4(xx, yy, zz, ww) {}
		static T Distance(const Quaternion& lhs, const Quaternion& rhs) {
			Quaternion conj = rhs;
			conj.Conjugate();
			return (lhs * conj).w();
		}

		Quaternion(const TMatrix<T, 4, 4>& m) { // from OGRE
			T tq[4];
			int i, j;

			// Use tq to store the largest trace
			tq[0] = 1 + m(0,0) + m(1,1) + m(2,2);
			tq[1] = 1 + m(0,0) - m(1,1) - m(2,2);
			tq[2] = 1 - m(0,0) + m(1,1) - m(2,2);
			tq[3] = 1 - m(0,0) - m(1,1) + m(2,2);

			// Find the maximum (could also use stacked if's later)
			j = 0;
			for (i = 1; i < 4; i++) j = (tq[i] > tq[j]) ? i : j;

			// check the diagonal
			if (j == 0) {
				/* perform instant calculation */
				this->w() = tq[0];
				this->x() = m(2,1) - m(1,2);
				this->y() = m(0,2) - m(2,0);
				this->z() = m(1,0) - m(0,1);
			} else if (j == 1) {
				this->w() = m(2,1) - m(1,2);
				this->x() = tq[1];
				this->y() = m(1,0) + m(0,1);
				this->z() = m(0,2) + m(2,0);
			} else if (j == 2) {
				this->w() = m(0,2) - m(2,0);
				this->x() = m(1,0) + m(0,1);
				this->y() = tq[2];
				this->z() = m(2,1) + m(1,2);
			} else {
				this->w() = m(1,0) - m(0,1);
				this->x() = m(0,2) + m(2,0);
				this->y() = m(2,1) + m(1,2);
				this->z() = tq[3];
			}

			Normalize();
		}

		static Quaternion Flip() {
			return Quaternion(0, 0, 0, 0);
		}

		static Quaternion Align(const Float3& from, const Float3& to) {
			const T EPSILON = (T)1e-3;
			TType3<T> axis = CrossProduct(to, from);
			T pcos = DotProduct(from, to);
			T halfcos = (T)sqrt(0.5 + pcos * 0.5);
			T ratio = halfcos > EPSILON ? (T)(0.5 / halfcos) : 0;

			return Quaternion(halfcos, axis.x() * ratio, axis.y() * ratio, axis.z() * ratio);
		}

		TType3<T> ToEulerAngle() const {
			T xx = (T)atan2(2 * (this->w() * this->x() + this->y() * this->z()), 1 - 2.0 * (this->x() * this->x() + this->y() * this->y()));
			T yy = (T)asin(2 * (this->w() * this->y() - this->z() * this->x()));
			T zz = (T)atan2(2 * (this->w() * this->z() + this->x() * this->y()), 1 - 2.0 * (this->y() * this->y() + this->z() * this->z()));

			return TType3<T>(xx, yy, zz);
		}

		Quaternion(const TType4<T>& quat) : Float4(quat) {}
		Quaternion(const TType3<T>& rot) {
			const T fSinPitch((T)sin(rot.y() * 0.5));
			const T fCosPitch((T)cos(rot.y() * 0.5));
			const T fSinYaw((T)sin(rot.z() * 0.5));
			const T fCosYaw((T)cos(rot.z() * 0.5));
			const T fSinRoll((T)sin(rot.x() * 0.5));
			const T fCosRoll((T)cos(rot.x() * 0.5));
			const T fCosPitchCosYaw(fCosPitch * fCosYaw);
			const T fSinPitchSinYaw(fSinPitch * fSinYaw);

			this->x() = fSinRoll * fCosPitchCosYaw - fCosRoll * fSinPitchSinYaw;
			this->y() = fCosRoll * fSinPitch * fCosYaw + fSinRoll * fCosPitch * fSinYaw;
			this->z() = fCosRoll * fCosPitch * fSinYaw - fSinRoll * fSinPitch * fCosYaw;
			this->w() = fCosRoll * fCosPitchCosYaw + fSinRoll * fSinPitchSinYaw;

			Normalize();
		}

		Quaternion& Normalize() {
			T mag = sqrt(this->x() * this->x() + this->y() * this->y() + this->z() * this->z() + this->w() * this->w());
			if (mag > 1e-6) {
				mag = 1.0f / mag;
				this->x() *= mag;
				this->y() *= mag;
				this->z() *= mag;
				this->w() *= mag;
			}

			return *this;
		}

		bool IsFlip() const {
			return this->x() == 0 && this->y() == 0 && this->z() == 0 && this->w() == 0;
		}

		Quaternion operator * (const Quaternion& t) const {
			assert(!this->IsFlip() && !t.IsFlip());
			return Quaternion(this->w() * t.w() - this->x() * t.x() - this->y() * t.y() - this->z() * t.z(),
				this->w() * t.x() + this->x() * t.w() + this->y() * t.z() - this->z() * t.y(),
				this->w() * t.y() + this->y() * t.w() + this->z() * t.x() - this->x() * t.z(),
				this->w() * t.z() + this->z() * t.w() + this->x() * t.y() - this->y() * t.x());
		}

		Quaternion& operator *= (const Quaternion& t) {
			*this = *this * t;
			return *this;
		}

		Quaternion& Conjugate() {
			this->x() = -this->x();
			this->y() = -this->y();
			this->z() = -this->z();

			return *this;
		}

		void Transform(TType3<T>& v) const {
			v = (*this)(v);
		}

		TType3<T> operator () (const TType3<T>& v) const {
			if (IsFlip()) {
				return TType3<T>(-v.x(), -v.y(), -v.z());
			} else {
				Quaternion q2(0, v.x(), v.y(), v.z()), q = *this, qinv = *this;
				q.Conjugate();

				q = q * q2 * qinv;
				return TType3<T>(q.x(), q.y(), q.z());
			}
		}

		static void Interpolate(Quaternion& out, const Quaternion& start, const Quaternion& end, T factor) {
			assert(!start.IsFlip() && !end.IsFlip());
			T cosom = start.x() * end.x() + start.y() * end.y() + start.z() * end.z() + start.w() * end.w();

			Quaternion qend = end;
			if (cosom < 0) {
				cosom = -cosom;
				qend.x() = -qend.x();
				qend.y() = -qend.y();
				qend.z() = -qend.z();
				qend.w() = -qend.w();
			}

			T sclp, sclq;
			if ((T)1.0 - cosom > 1e-6) {
				T omega, sinom;
				omega = acos(cosom);
				sinom = sin(omega);
				sclp = sin((1.0f - factor) * omega) / sinom;
				sclq = sin(factor * omega) / sinom;
			} else {
				sclp = (T)1.0 - factor;
				sclq = factor;
			}

			out.x() = sclp * start.x() + sclq * qend.x();
			out.y() = sclp * start.y() + sclq * qend.y();
			out.z() = sclp * start.z() + sclq * qend.z();
			out.w() = sclp * start.w() + sclq * qend.w();
		}

		static void InterpolateSquad(Quaternion& out, const Quaternion& left, const Quaternion& outTan,  const Quaternion& right, const Quaternion& inTan, float factor) {
			T t = (T)(2.0 * factor * (1.0 - factor));
			Quaternion p, q;
			Interpolate(p, left, right, t);
			Interpolate(q, outTan, inTan, t);

			Interpolate(out, p, q, t);
		}

		void WriteMatrix(TMatrix<T, 4U, 4U>& m) const {
			if (!IsFlip()) {
				T mat[16];
				mat[0] = 1 - 2 * (this->y() * this->y() + this->z() * this->z());
				mat[1] = 2 * (this->x() * this->y() - this->z() * this->w());
				mat[2] = 2 * (this->x() * this->z() + this->y() * this->w());
				mat[3] = 0;
				mat[4] = 2 * (this->x() * this->y() + this->z() * this->w());
				mat[5] = 1 - 2 * (this->x() * this->x() + this->z() * this->z());
				mat[6] = 2 * (this->y() * this->z() - this->x() * this->w());
				mat[7] = 0;
				mat[8] = 2 * (this->x() * this->z() - this->y() * this->w());
				mat[9] = 2 * (this->y() * this->z() + this->x() * this->w());
				mat[10] = 1 - 2 * (this->x() * this->x() + this->y() * this->y());
				mat[11] = mat[12] = mat[13] = mat[14] = 0;
				mat[15] = 1;

				m = TMatrix<T, 4U, 4U>(mat);
			} else {
				T mat[16] = { -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
				m = TMatrix<T, 4U, 4U>(mat);
			}
		}
	};
}

