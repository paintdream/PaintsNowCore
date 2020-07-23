// Tape.h
// By PaintDream (paintdream@paintdream.com)
// 2015-1-8
//

#ifndef __TAPE_H__ 
#define __TAPE_H__

#include "../../Core/Interface/IStreamBase.h"
#include "../../Core/Interface/IType.h"

namespace PaintsNow {
	class Tape {
	private:
		struct PacketHeader {
		public:
			int64_t firstMark : 1;
			int64_t padding : 5;
			int64_t offset : 5; // log2(32)
			int64_t seq : 53; // 64 - firstMark - offset - padding
			// uint32_t length; // if firstMark = 1
		};

		// 32-bit version
		/* struct PacketHeader {
		public:
			uint32_t firstMark : 1;
			uint32_t offsetType : 1;
			uint32_t offset : 5;
			uint32_t seq : 25;
		};*/
	public:
		Tape(IStreamBase& stream);
		virtual ~Tape();

		enum { PACKAGE_MIN_ALIGN_LEVEL = 5 };
		enum { PACKAGE_MIN_ALIGN = 1 << (PACKAGE_MIN_ALIGN_LEVEL) };

		virtual bool ReadPacket(int64_t& seq, IStreamBase& target, uint32_t& length);
		virtual bool WritePacket(int64_t seq, IStreamBase& source, uint32_t length);
		virtual bool Seek(int64_t seq);
		int64_t GetMaxLocation() const;

		// static int main(int argc, char* argv[]);
	//	int64_t totalPadding;

	protected:
		int FullLength(int64_t location, int64_t& length);
		IStreamBase& stream;
		int64_t location;
		int64_t maxLocation;
		char lastOffset;
	};
}


#endif // __TAPE_H__
