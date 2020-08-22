// ShadowStream.h
// By PaintDream (paintdream@paintdream.com)
// 2015-1-19
//

#pragma once
#include "../../Core/Interface/IType.h"
#include "../../Core/Interface/IStreamBase.h"
#include "../../Core/Template/TBuffer.h"

namespace PaintsNow {
	class ShadowStream : public IStreamBase {
	public:
		ShadowStream();
		virtual ~ShadowStream();

		virtual bool operator << (IStreamBase& stream) override;
		virtual bool operator >> (IStreamBase& stream) const override;

		virtual bool Read(void* p, size_t& len) override;
		virtual bool Write(const void* p, size_t& len) override;
		virtual bool WriteDummy(size_t& len) override;
		virtual bool Seek(SEEK_OPTION option, int64_t offset) override;
		virtual void Flush() override;
		virtual bool Transfer(IStreamBase& stream, size_t& len) override;

		Bytes& GetPayload();
		const Bytes& GetPayload() const;

		ShadowStream& operator = (const ShadowStream& localStream);
	
	protected:
		void Cleanup();

		IStreamBase* baseStream;
		uint64_t length;
		Bytes payload;
	};
}

