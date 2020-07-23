#include "MemoryStream.h"
#include "../../Core/Interface/IMemory.h"

using namespace PaintsNow;

MemoryStream::MemoryStream(size_t b, bool s, uint16_t align) : offset(0), totalSize(0), maxSize(b), autoSize(s), alignment(align) {
	buffer = (uint8_t*)IMemory::AllocAligned(maxSize, alignment);
}

void MemoryStream::Realloc(size_t newSize) {
	assert(newSize > maxSize);
	size_t newMaxSize = maxSize * 2;
	while (newMaxSize < newSize) newMaxSize <<= 1;

	uint8_t* w = (uint8_t*)IMemory::AllocAligned(newMaxSize, alignment);
	memcpy(w, buffer, totalSize);
	std::swap(buffer, w);

	IMemory::FreeAligned(w);
	maxSize = newMaxSize;
	totalSize = newSize;
}

MemoryStream::~MemoryStream() {
	IMemory::FreeAligned(buffer);
}

const void* MemoryStream::GetBuffer() const {
	return buffer;
}

void MemoryStream::Clear() {
	offset = 0;
	totalSize = 0;
}

bool MemoryStream::Read(void* p, size_t& len) {
	// printf("READ OFFSET: %d\n", offset);
	if (len + offset > totalSize) {
		len = totalSize - offset;
		return false;
	}

	memcpy(p, buffer + offset, len);
	offset += len;

	return true;
}

bool MemoryStream::CheckSize(size_t& len) {
	if (len + offset > totalSize) {
		if (len + offset <= maxSize) {
			totalSize = len + offset;
		} else if (autoSize) {
			Realloc(len + offset);
		} else {
			len = totalSize - offset;
			return false;
		}
	}

	return true;
}

IReflectObject* MemoryStream::Clone() const {
	// Not clonable by now
	// TODO:
	assert(false);
	return nullptr;
}

bool MemoryStream::Transfer(IStreamBase& stream, size_t& len) {
	if (!CheckSize(len)) {
		return false;
	}

	stream.Write(buffer + offset, len);
	offset += len;

	totalSize = Math::Max(totalSize, offset);
	return true;
}

void* MemoryStream::GetBuffer() {
	return buffer;
}

bool MemoryStream::Extend(size_t len) {
	if (!CheckSize(len)) {
		return false;
	}

	totalSize = Math::Max(totalSize, (size_t)offset + len);
	return true;
}

bool MemoryStream::WriteDummy(size_t& len) {
	if (!CheckSize(len)) {
		return false;
	}

	offset += len;
	totalSize = Math::Max(totalSize, (size_t)offset);
	return true;
}

bool MemoryStream::Write(const void* p, size_t& len) {
	// printf("WRITE OFFSET: %d\n", offset);
	if (!CheckSize(len)) {
		return false;
	}

	memcpy(buffer + offset, p, len);
	offset += len;
	totalSize = Math::Max(totalSize, (size_t)offset);

	return true;
}

void MemoryStream::Flush() {

}

void MemoryStream::SetEnd() {
	totalSize = offset;
}

bool MemoryStream::Seek(SEEK_OPTION option, long f) {
	switch (option) {
	case IStreamBase::BEGIN:
		if (f < 0 || (size_t)f > totalSize)
			return false;
		offset = (size_t)f;
		break;
	case IStreamBase::CUR:
		if (offset + f < 0 || (size_t)(offset + f) > totalSize)
			return false;
		offset += f;
		break;
	case IStreamBase::END:
		offset = totalSize;
		break;
	}

	return true;
}


size_t MemoryStream::GetOffset() const {
	return offset;
}

size_t MemoryStream::GetTotalLength() const {
	return totalSize;
}

size_t MemoryStream::GetMaxLength() const {
	return maxSize;
}
