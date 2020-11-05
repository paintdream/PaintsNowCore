// IArchive.h -- Archive format supporting
// By PaintDream (paintdream@paintdream.com)
// 2014-12-1
//

#pragma once

#include "../../Core/PaintsNow.h"
#include "../../Core/Interface/IDevice.h"
#include "../../Core/Template/TProxy.h"
#include "../../Core/Interface/IType.h"

namespace PaintsNow {
	class IStreamBase;
	class IArchive : public IDevice {
	public:
		~IArchive() override;

		virtual String GetFullPath(const String& path) const = 0;
		virtual bool Mount(const String& prefix, IArchive* baseArchive = nullptr) = 0;
		virtual bool Unmount(const String& prefix, IArchive* baseArchive = nullptr) = 0;
		virtual bool Exists(const String& path) const = 0;
		virtual IStreamBase* Open(const String& path, bool write, uint64_t& length, uint64_t* lastModifiedTime = nullptr) = 0;
		virtual void Query(const String& uri, const TWrapper<void, const String&>& wrapper) const = 0;
		virtual bool IsReadOnly() const = 0;
		virtual bool Delete(const String& uri) = 0;
	};
}

