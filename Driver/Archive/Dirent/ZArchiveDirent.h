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
		ZArchiveDirent(const String& root = ".", bool sandBox = false);
		virtual ~ZArchiveDirent();

		virtual const String& GetRootPath() const;
		virtual void SetRootPath(const String& path);

		virtual IStreamBase* Open(const String& uri, bool write, size_t& length, uint64_t* lastModifiedTime = nullptr);
		virtual void Query(const String& uri, const TWrapper<void, bool, const String&>& wrapper) const;
		virtual bool IsReadOnly() const;
		virtual bool Delete(const String& uri);

	private:
		bool FilterSandBox(const String& uri) const;
		bool MakeDirectoryForFile(const String& path);

	private:
		String root;
		bool sandBoxEnabled;
		bool reserved[3];
	};
}

