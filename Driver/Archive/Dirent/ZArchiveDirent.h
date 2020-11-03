// ZArchiveDirent -- Basic file system management by dirent and standard c interfaces.
// By PaintDream (paintdream@paintdream.com)
//

#pragma once
#include "../../../Interface/IArchive.h"
#include "../../../../Core/Interface/IStreamBase.h"
#include <iconv.h>

namespace PaintsNow {
	class ZArchiveDirent final : public IArchive {
	public:
		ZArchiveDirent(const String& root = "./");
		~ZArchiveDirent() override;

		bool Mount(const String& prefix, IArchive* baseArchive) override;
		bool Unmount(const String& prefix, IArchive* baseArchive) override;

		IStreamBase* Open(const String& uri, bool write, size_t& length, uint64_t* lastModifiedTime = nullptr) override;
		void Query(const String& uri, const TWrapper<void, bool, const String&>& wrapper) const override;
		bool IsReadOnly() const override;
		bool Delete(const String& uri) override;

	private:
		bool MakeDirectoryForFile(const String& path);

	private:
		String root;
	};
}

