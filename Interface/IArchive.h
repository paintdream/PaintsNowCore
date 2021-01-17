// IArchive.h -- Archive format supporting
// PaintDream (paintdream@paintdream.com)
// 2014-12-1
//

#pragma once

#include "../PaintsNow.h"
#include "../Interface/IDevice.h"
#include "../Template/TProxy.h"
#include "../Interface/IType.h"

namespace PaintsNow {
	class IStreamBase;
	class pure_interface IArchive : public IDevice {
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

