// TTagged.h
// Tagged pointer
// PaintDream (paintdream@paintdream.com)
//

#ifndef __TTAGGED_H__
#define __TTAGEED_H__

#include "../Interface/IType.h"

template <class T, size_t N>
class TTagged {
public:
	typedef T Type;
	enum { MASK = (1 << N) - 1 };

	TTagged(T ptr = nullptr, size_t tag = 0) : storage(reinterpret_cast<T>((size_t)ptr | tag)) {
		assert(((size_t)ptr & MASK) == 0);
		assert(tag <= MASK);
	}

	size_t Tag() const {
		return (size_t)storage & MASK;
	}

	void Tag(size_t tag) {
		assert(tag <= MASK);
		storage = reinterpret_cast<T>(((size_t)storage & ~MASK) | tag);
	}

	void operator () (T t) {
		assert(((size_t)t & MASK) == 0);
		storage = reinterpret_cast<T>((size_t)t | ((size_t)storage & MASK));
	}

	const T operator () () const {
		return reinterpret_cast<T>((size_t)storage & ~MASK);
	}

	T operator () () {
		return reinterpret_cast<T>((size_t)storage & ~MASK);
	}

private:
	T storage;
};

#endif // __TTAGGED_H__
