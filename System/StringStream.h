// StringStream.h
// By PaintDream (paintdream@paintdream.com)
// 2015-1-19
//

#ifndef __STRINGSTREAM_H__
#define __STRINGSTREAM_H__

#include "../../Core/Interface/IType.h"
#include "../../Core/Interface/IStreamBase.h"

namespace PaintsNow {
	class StringStream : public IStreamBase {
	public:
		StringStream(String& str, int64_t pos = 0);
		virtual ~StringStream();

		virtual bool Read(void* p, size_t& len) override;
		virtual bool Write(const void* p, size_t& len) override;
		virtual bool WriteDummy(size_t& len) override;
		virtual bool Seek(SEEK_OPTION option, int64_t offset) override;
		virtual void Flush() override;
		virtual bool Transfer(IStreamBase& stream, size_t& len) override;
	
	protected:
		int64_t location;
		String& storage;
	};
}

#endif // __STRINGSTREAM_H__