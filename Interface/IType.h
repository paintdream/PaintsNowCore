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
#include <cmath>
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

#include "../Backport/VC98STRING.h"
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
	typedef std::string_mt String;
	std::string Utf8ToStd(const String& str);
	String StdToUtf8(const std::string& str);
#else
	typedef std::string String;
#define StdToUtf8(f) (f)
#define Utf8ToStd(f) (f)
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
#if defined(_MSC_VER) && _MSC_VER <= 1200
	typedef __declspec(align(16)) TType4<float> Float4;
#else
	typedef TType4<float> Float4;
#endif
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
#if defined(_MSC_VER) && _MSC_VER <= 1200
	typedef __declspec(align(16)) TMatrix<float, 4U, 4U> MatrixFloat4x4;
#else
	typedef TMatrix<float, 4U, 4U> MatrixFloat4x4;
#endif
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
		MatrixFloat4x4 Rotate3D(const MatrixFloat4x4& input, float rad, const Float3& d);
		MatrixFloat4x4 Translate3D(const MatrixFloat4x4& input, const Float3& v);
		Float3 Transform3D(const MatrixFloat4x4& input, const Float3& v);
		MatrixFloat4x4 LookAt(const Float3& position, const Float3& dir, const Float3& u);
		MatrixFloat4x4 Ortho(const Float3& size);
		MatrixFloat4x4 Perspective(float d, float rr, float n, float f);
		MatrixFloat4x4 InverseProjection(const MatrixFloat4x4& m);

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

}

