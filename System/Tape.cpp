#include "Tape.h"
#include "../../Core/Template/TAlgorithm.h"
#include <cassert>

using namespace PaintsNow;

Tape::Tape(IStreamBase& s) : stream(s), location(0), maxLocation(0), lastOffset(0)/*, totalPadding(0)*/ {
}

Tape::~Tape() {
}

#ifndef _DEBUG
#define verify(f) (f);
#else
#define verify(f) \
	assert((f));
#endif

inline int Tape::FullLength(int64_t location, int64_t& length) {
	for (int c = 0, d; c < 32; c++) {
		if (c >= (d = (int)Math::SiteCount(location >> PACKAGE_MIN_ALIGN_LEVEL, (int64_t)((location + length + c * sizeof(PacketHeader) + PACKAGE_MIN_ALIGN - 1) >> PACKAGE_MIN_ALIGN_LEVEL)))) {
			length += c * sizeof(PacketHeader);
			// printf("Site Count %d\n", d);
			return c - d;
		}
	}

	assert(false);
	return 0;
}

bool Tape::WritePacket(int64_t seq, IStreamBase& source, uint32_t length) {
	// static uint8_t filldummy[PACKAGE_MIN_ALIGN - sizeof(PacketHeader)];
	// find aligned point
	int64_t fullLength = (int64_t)length + sizeof(length);
	int padding = FullLength(location, fullLength);
//	totalPadding += (sizeof(PacketHeader) << padding);
	PacketHeader header;
	header.firstMark = 1;
	header.padding = padding;
	header.seq = seq;
//	int64_t savedLocation = location;

	int64_t alignedEnd = location + ((fullLength + PACKAGE_MIN_ALIGN - 1) & ~(PACKAGE_MIN_ALIGN - 1));
	int64_t mid = Math::AlignmentRange(location, alignedEnd);
	int64_t remaining = alignedEnd;

 	while (length > 0) {
		uint32_t alignment = safe_cast<uint32_t>(location < mid ? Math::Alignment(location | mid) : Math::AlignmentTop(remaining - mid));
		if (location >= mid) remaining -= alignment;

		//printf("Header at %p offset = %d, alignment = %p\n", (uint32_t)location, lastOffset, alignment);
		assert(alignment != 0);
		// write atmost alignment - sizeof(header?) bytes
		uint32_t extra = header.firstMark ? sizeof(length) : 0;
		uint32_t full = alignment - sizeof(PacketHeader) - extra;
		uint32_t size = Math::Min(length, full);

		header.offset = lastOffset;

		// write header
		size_t wl = sizeof(PacketHeader);
		if (!stream.Write((const uint8_t*)&header, wl)) {
			return false;
		}

		// write length for first packet
		uint32_t len = safe_cast<uint32_t>(fullLength);
		wl = sizeof(len);
		if (extra != 0 && !stream.Write((const uint8_t*)&len, wl)) {
			return false;
		}

		// prepare data
		wl = size;
		if (!source.Transfer(stream, wl)) {
			return false;
		}

		header.firstMark = 0;
		length -= size;
		location += alignment;
		lastOffset = Math::Log2(alignment);

		if (length == 0) {
			wl = (size_t)(alignedEnd - location + full - size);
			if (!stream.WriteDummy(wl)) {
				return false;
			}

			location = alignedEnd;
		}
	}

	/*
	long diff = (long)Math::Alignment(location);
	if (!stream.Seek(IStreamBase::CUR, -diff)) {
		return false;
	}

	if (!stream.Read(&header, sizeof(header))) {
		return false;
	}

	assert(header.seq != 0);
	if (!stream.Seek(IStreamBase::CUR, -(long)sizeof(header) + diff)) {
		return false;
	}*/

//	assert(location == stream.GetOffset());
	maxLocation = location;
	return true;
}

bool Tape::ReadPacket(int64_t& seq, IStreamBase& target, uint32_t& totalLength) {
	if (location >= maxLocation)
		return false;

	PacketHeader header;
	uint32_t length = 0;
	totalLength = 0;
	// bool type = false;
	int64_t alignedEnd = 0;
	int64_t mid = 0;
	int64_t remaining = 0;

	do {
		size_t rl = sizeof(PacketHeader);
		if (!stream.Read(&header, rl)) {
			return false;
		}

		// assert(header.firstMark);

		if (length == 0) {
			assert(header.firstMark);
			rl = sizeof(length);
			if (!stream.Read(&length, rl)) {
				return false;
			} else {
				seq = header.seq;
				// totalLength = length;
				alignedEnd = location + ((length + PACKAGE_MIN_ALIGN - 1) & ~(PACKAGE_MIN_ALIGN - 1));
				length -= safe_cast<uint32_t>(header.padding * sizeof(PacketHeader));
				mid = Math::AlignmentRange(location, alignedEnd);
				remaining = alignedEnd;
			}
		}

		uint32_t alignment = safe_cast<uint32_t>(location < mid ? Math::Alignment(location | mid) : Math::AlignmentTop(remaining - mid));
		if (location >= mid) remaining -= alignment;
		uint32_t extra = header.firstMark ? sizeof(length) : 0;
		assert(length > sizeof(PacketHeader) + extra);
		uint32_t size = Math::Min(length, alignment) - (sizeof(PacketHeader) + extra);

		assert((int64_t)size > 0);
		// read content
		rl = size;
		if (!stream.Transfer(target, rl)) {
			return false;
		}

		if (!stream.Seek(IStreamBase::CUR, alignment - size - sizeof(PacketHeader) - extra)) {
			return false;
		}

		totalLength += size;
		length -= size + sizeof(PacketHeader) + extra;
		location += alignment;
//		assert(location == stream.GetOffset());
		lastOffset = header.offset;
	} while (length > 0);

	return true;
}
