// TBuffer -- Basic buffer structure
// By PaintDream (paintdream@paintdream.com)
// 2014-11-30
//

#pragma once
#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include <string>

namespace PaintsNow {
	// Memory buffer with some internal storage.
	template <class T, size_t N = (sizeof(T*) * 4 - sizeof(size_t)) / sizeof(T)>
	class TBuffer {
	public:
		enum
#if !defined(_MSC_VER) || _MSC_VER > 1200
			: size_t
#endif
		{
			EXT_STORE_MASK = ((size_t)1 << (sizeof(size_t) * 8 - 1))
		};

		TBuffer() : size(0) {}

		TBuffer(size_t initSize) : size(0) {
			if (initSize != 0) {
				Resize(initSize);
			}
		}

		TBuffer(const T* p, size_t initSize) : size(0) {
			if (initSize != 0) {
				Resize(initSize);
				memcpy(GetData(), p, initSize * sizeof(T));
			}
		}

		TBuffer(const TBuffer& rhs) : size(0) {
			Copy(rhs);
		}

		~TBuffer() {
			if (!IsStockStorage()) {
				assert(buffer != nullptr);
				free(buffer);
			}
		}

		void Clear() {
			if (!IsStockStorage()) {
				assert(buffer != nullptr);
				free(buffer);
			}

			size = 0;
		}

		TBuffer& operator = (const std::basic_string<T, std::char_traits<T>, std::allocator<T> >& str) {
			Resize(str.size());
			memcpy(GetData(), str.data(), str.size() * sizeof(T));
			return *this;
		}

		TBuffer& operator = (const TBuffer& rhs) {
			Copy(rhs);
			return *this;
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		explicit
#endif
		TBuffer(rvalue<TBuffer> rv) {
			TBuffer& rhs = rv;
			memcpy(this, &rhs, sizeof(*this));
			rhs.size = 0;
		}

		TBuffer& operator = (rvalue<TBuffer> rv) {
			Clear();

			TBuffer& rhs = rv;
			memcpy(this, &rhs, sizeof(*this));
			rhs.size = 0;

			return *this;
		}

		static TBuffer& Null() {
			static TBuffer empty;
			return empty;
		}

		bool IsStockStorage() const { return !(size & EXT_STORE_MASK); }
		size_t GetSize() const { assert(size <= N || (size & ~EXT_STORE_MASK) > N); return size & ~EXT_STORE_MASK; }
		const T* GetData() const { return IsStockStorage() ? stockStorage : buffer; }
		T* GetData() { return IsStockStorage() ? stockStorage : buffer; }

		bool Empty() const { return size == 0; }
		bool operator == (const TBuffer& rhs) const {
			if (size != rhs.size) return false;
			if (size == 0) return true;
			return memcmp(GetData(), rhs.GetData(), GetSize() * sizeof(T)) == 0;
		}

		bool operator < (const TBuffer& rhs) const {
			if (size == 0) {
				return rhs.size != 0;
			} else {
				bool less = size < rhs.size;
				size_t minSize = (less ? size : rhs.size) & (~EXT_STORE_MASK);
				int result = memcmp(GetData(), rhs.GetData(), minSize * sizeof(T));
				return result != 0 ? result < 0 : less;
			}
		}

		const T& operator [] (size_t index) const {
			assert(index < GetSize());
			return GetData()[index];
		}

		T& operator [] (size_t index) {
			assert(index < GetSize());
			return GetData()[index];
		}

		void Resize(size_t s, const T& init) {
			size_t orgSize = GetSize();
			Resize(s);

			if (s > orgSize) {
				T* ptr = GetData();
				std::fill(ptr + orgSize, ptr + s, init);
			}
		}

		void Resize(size_t s) {
			if (IsStockStorage()) {
				if (s > N) { // out of bound
					T* newBuffer = reinterpret_cast<T*>(malloc(s * sizeof(T)));
					memcpy(newBuffer, stockStorage, GetSize() * sizeof(T));
					buffer = newBuffer;
					size = s | EXT_STORE_MASK;
				} else {
					size = s;
				}
			} else {
				if (s > N) {
					if (s > GetSize()) {
						buffer = reinterpret_cast<T*>(realloc(buffer, s * sizeof(T)));
					}

					size = s | EXT_STORE_MASK;
				} else {
					T* orgBuffer = buffer;
					memcpy(stockStorage, orgBuffer, s * sizeof(T));
					free(orgBuffer);

					size = s;
				}
			}
		}

		void Swap(TBuffer& rhs) {
			std::swap(size, rhs.size);
			for (size_t i = 0; i < N; i++) {
				std::swap(stockStorage[i], rhs.stockStorage[i]);
			}
		}

		TBuffer& Append(const TBuffer& rhs) {
			return Append(rhs.GetData(), rhs.GetSize());
		}

		TBuffer& Append(const T* buffer, size_t appendSize) {
			if (appendSize != 0) {
				size_t orgSize = GetSize();
				Resize(orgSize + appendSize);
				memcpy(GetData() + orgSize, buffer, appendSize * sizeof(T));
			}

			return *this;
		}

		TBuffer& Assign(const T* buffer, size_t n) {
			Resize(n);
			if (n != 0) {
				memcpy(GetData(), buffer, n * sizeof(T));
			}

			return *this;
		}

	protected:
		void Copy(const TBuffer& rhs) {
			size_t s = rhs.GetSize();
			Resize(s);
			memcpy(GetData(), rhs.GetData(), s * sizeof(T));
		}

		size_t size;
		union {
			T* buffer;
			T stockStorage[N];
		};
	};

	typedef TBuffer<uint8_t> Bytes;

#define StaticBytes(f) Bytes((const uint8_t*)#f, sizeof(#f) - 1)
}

