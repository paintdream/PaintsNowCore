#include "IType.h"
#include <cmath>
#include <iostream>
#include <cfloat>

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
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

#if defined(_MSC_VER) && _MSC_VER <= 1200
	String StdToUtf8(const std::string& str) {
		return String(str.c_str(), str.size());
	}

	std::string Utf8ToStd(const String& str) {
		return std::string(str.c_str(), str.size());
	}
#endif

#ifdef _WIN32
	String Utf8ToSystem(const String& str) {
		DWORD dwMinSize;
		dwMinSize = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
		String ret;
		ret.resize((size_t)dwMinSize * sizeof(WCHAR) + sizeof(WCHAR), 0);
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
