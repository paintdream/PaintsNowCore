// TBuffer -- Basic buffer structure
// By PaintDream (paintdream@paintdream.com)
// 2014-11-30
//

#ifndef __TBUFFER_H__
#define __TBUFFER_H__

#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include <string>

namespace PaintsNow {
	template <class T, uint32_t N = (sizeof(T*) * 4 - sizeof(uint32_t)) / sizeof(T)>
	class TBuffer {
	public:
		enum { STOCK_MASK = (uint32_t)1 << (sizeof(uint32_t) * 8 - 1) };
		TBuffer(uint32_t initSize = 0) {
			memset(this, 0, sizeof(*this));
			if (initSize != 0) {
				Resize(initSize);
			}
		}

		TBuffer(const TBuffer& rhs) {
			memset(this, 0, sizeof(*this));
			Copy(rhs);
		}

		~TBuffer() {
			Clear();
		}

		inline void Clear() {
			if (!IsStockStorage() && buffer != nullptr) {
				free(buffer);
			}

			size = 0;
			buffer = nullptr;
		}

		inline TBuffer& operator = (const std::basic_string<T, std::char_traits<T>, std::allocator<T> >& str) {
			Clear();

			Resize(str.size());
			memcpy(GetData(), str.data(), str.size());
		}

		TBuffer& operator = (const TBuffer& rhs) {
			Copy(rhs);
			return *this;
		}

		TBuffer(rvalue<TBuffer> rv) {
			TBuffer& rhs = rv;
			memcpy(this, &rhs, sizeof(*this));
			memset(&rhs, 0, sizeof(*this));
		}

		TBuffer& operator = (rvalue<TBuffer> rv) {
			Clear();

			TBuffer& rhs = rv;
			memcpy(this, &rhs, sizeof(*this));
			memset(&rhs, 0, sizeof(*this));

			return *this;
		}

		static inline TBuffer& Null() {
			static TBuffer empty;
			return empty;
		}

		inline uint32_t GetSize() const { return size & ~STOCK_MASK; }
		inline const T* GetData() const { return IsStockStorage() ? stockStorage : buffer; }
		inline T* GetData() { return Empty() ? nullptr : IsStockStorage() ? stockStorage : buffer; }

		inline bool Empty() const { return GetSize() == 0; }
		inline bool operator == (const TBuffer& rhs) const {
			uint32_t size = GetSize();
			if (size != rhs.GetSize()) return false;
			if (size == 0) return true;

			return memcmp(GetData(), rhs.GetData(), size * sizeof(T)) == 0;
		}

		inline bool operator < (const TBuffer& rhs) const {
			uint32_t size = GetSize();
			if (size == 0) {
				return rhs.GetSize() != 0;
			} else {
				uint32_t minSize = Min(size, rhs.GetSize());
				int result = memcmp(GetData(), rhs.GetData(), minSize * sizeof(T));
				return result != 0 ? result < 0 : size < minSize;
			}
		}

		inline void Resize(uint32_t s, const T& init) {
			uint32_t orgSize = GetSize();
			Resize(s);

			if (s > orgSize) {
				T* ptr = GetData();
				std::fill(ptr + orgSize, ptr + s, init);
			}
		}

		inline void Resize(uint32_t s) {
			if (IsStockStorage()) {
				if (s > N) { // out of bound
					T* newBuffer = reinterpret_cast<T*>(malloc(s * sizeof(T)));
					memcpy(newBuffer, stockStorage, GetSize() * sizeof(T));
					buffer = newBuffer;
					size = s;
				} else {
					size = s | STOCK_MASK;
				}
			} else {
				if (s > N) {
					if (s > size) {
						buffer = reinterpret_cast<T*>(realloc(buffer, s * sizeof(T)));
					}

					size = s;
				} else {
					T* orgBuffer = buffer;
					if (orgBuffer != nullptr) {
						memcpy(stockStorage, orgBuffer, s * sizeof(T));
						free(orgBuffer);
					}

					size = s | STOCK_MASK;
				}
			}
		}

		inline void Swap(TBuffer& rhs) {
			std::swap(size, rhs.size);
			T t[N];
			memcpy(t, stockStorage, sizeof(T) * N);
			memcpy(stockStorage, rhs.stockStorage, sizeof(T) * N);
			memcpy(rhs.stockStorage, t, sizeof(T) * N);
		}

		inline TBuffer& Append(const TBuffer& rhs) {
			return Append(rhs.GetData(), rhs.GetSize());
		}

		inline TBuffer& Append(const T* buffer, uint32_t appendSize) {
			if (appendSize != 0) {
				uint32_t orgSize = GetSize();
				Resize(orgSize + appendSize);
				memcpy(GetData() + orgSize, buffer, appendSize);
			}

			return *this;
		}

		inline TBuffer& Assign(const T* buffer, uint32_t n) {
			Resize(n);
			if (n != 0) {
				memcpy(GetData(), buffer, n);
			}

			return *this;
		}

		inline bool IsStockStorage() const { return !!(size & STOCK_MASK); }
	protected:
		inline void Copy(const TBuffer& rhs) {
			uint32_t s = rhs.GetSize();
			Resize(s);
			memcpy(GetData(), rhs.GetData(), s);
		}

		union {
			T* buffer;
			T stockStorage[N];
		};

		uint32_t size;
	};

	typedef TBuffer<uint8_t> Bytes;

	template <class T, size_t N>
	struct TBlock {
		enum { size = N};
		TBlock() : count(0) {}

		uint32_t count;
		T data[N];
	};
}

#endif // __TBUFFER_H__
