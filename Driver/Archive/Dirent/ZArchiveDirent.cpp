#include "ZArchiveDirent.h"
#include "../../../../Core/Template/TAlgorithm.h"
#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#endif
#include <string>

using namespace PaintsNow;

class FileStream : public IStreamBase {
public:
	FileStream(FILE* p, rvalue<String> path) : fp(p), filePath(std::move(path)) {

	}
	virtual ~FileStream() { fclose(fp);}

	virtual IReflectObject* Clone() const {
		FILE* fp = fopen(filePath.c_str(), "rb");
		if (fp != nullptr) {
			return new FileStream(fp, String(filePath));
		} else {
			return nullptr;
		}
	}

	virtual bool Read(void* p, size_t& len) {
		return (len = fread(p, 1, len, fp)) != 0;
	}

	virtual bool Write(const void* p, size_t& len) {
		return (len = fwrite(p, 1, len, fp)) != 0;
	}

	virtual bool Seek(SEEK_OPTION option, int64_t offset) {
		int s = SEEK_CUR;
		switch (option) {
		case BEGIN:
			s = SEEK_SET;
			break;
		case END:
			s = SEEK_END;
			break;
		case CUR:
			s = SEEK_CUR;
			break;
		}

		assert(abs(offset) < 0x7FFFFFFF);
		return fseek(fp, (long)offset, s) == 0;
	}

	virtual bool Transfer(IStreamBase& stream, size_t& len) {
		const size_t SIZE = 512;
		char buffer[SIZE];
		size_t rl = Math::Min(SIZE, len);
		while (len > 0 && Read(buffer, rl)) {
			size_t wl = Math::Min(SIZE, len);
			stream.Write(buffer, wl);
			len -= SIZE;
			rl = Math::Min(SIZE, len);
		}

		return len == 0;
	}

	virtual bool WriteDummy(size_t& len) {
		return fseek(fp, (long)len, SEEK_CUR) == 0;
	}

	virtual long GetOffset() const {
		return ftell(fp);
	}

	virtual void Flush() {
		fflush(fp);
	}

private:
	FILE* fp;
	String filePath;
};

const String& ZArchiveDirent::GetRootPath() const {
	return root;
}

void ZArchiveDirent::SetRootPath(const String& path) {
	root = path;
	if (root != "" && (root[root.length() - 1] != '\\' && root[root.length() - 1] != '/'))
		root += '/';
}

ZArchiveDirent::ZArchiveDirent(const String& r, bool sandBox) : sandBoxEnabled(sandBox) {
	SetRootPath(r.length() == 0 ? String(".") : r);
}

ZArchiveDirent::~ZArchiveDirent() {
}

bool ZArchiveDirent::IsReadOnly() const {
	return false;
}

bool ZArchiveDirent::FilterSandBox(const String& uri) const {
	if (sandBoxEnabled && uri.find("..") != String::npos) {
		return true;
	} else {
		return false;
	}
}

#ifdef _WIN32
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

BOOL DirectoryExists(LPCWSTR szPath) {
	DWORD dwAttrib = GetFileAttributesW(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool MakeDirectoryRecursive(const wchar_t* path) {
	wchar_t fullPath[MAX_PATH * 4];
	wcsncpy(fullPath, path, MAX_PATH * 4);
	if (!PathRemoveFileSpecW(fullPath)) return false;
	if (!DirectoryExists(fullPath)) {
		if (path[0] == '\0') return false;
		if (MakeDirectoryRecursive(fullPath)) {
			return ::CreateDirectoryW(fullPath, nullptr) != FALSE;
		} else {
			return false;
		}
	} else {
		return true;
	}
}

#else
bool MakeDirectoryRecursive(const String& filePath) {
	String path = filePath.substr(0, filePath.find_last_of('/'));
	bool ret = false;
	int state = mkdir(path.c_str(), 0775);
	if (state == -1) {
		if (errno == ENOENT) {
			if (MakeDirectoryRecursive(path)) {
				// recreate
				state = mkdir(path.c_str(), 0775);
			}
		} else if (errno == EEXIST) {
			state = true;
		}
	} else {
		state = true;
	}

	return state;
}

#endif

bool ZArchiveDirent::MakeDirectoryForFile(const String& filePath) {
	#ifdef _WIN32
	WCHAR fullPath[MAX_PATH * 4];
	::GetFullPathNameW((wchar_t*)filePath.c_str(), MAX_PATH * 4, fullPath, nullptr);
	return MakeDirectoryRecursive(fullPath);
	#else
	return MakeDirectoryRecursive(filePath);
	#endif
}

IStreamBase* ZArchiveDirent::Open(const String& uri, bool write, size_t& length, uint64_t* lastModifiedTime) {
	if (FilterSandBox(uri)) {
		return nullptr;
	}

	String path = root + uri;

#if defined(_WIN32) || defined(WIN32)
	String wpath = Utf8ToSystem(path);
	if (write) {
		if (!MakeDirectoryForFile(wpath)) {
			return nullptr;
		}
	}

	FILE* fp = _wfopen((const wchar_t*)wpath.c_str(), write ? L"wb" : L"rb");
	if (fp == nullptr) {
		DWORD err = GetLastError();
		return nullptr;
	}

	int fd = _fileno(fp);
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile != INVALID_HANDLE_VALUE) {
		if (lastModifiedTime != nullptr) {
			FILETIME ftLastModified;
			if (::GetFileTime(hFile, nullptr, nullptr, &ftLastModified)) {
				uint64_t m = ((uint64_t)ftLastModified.dwHighDateTime << 32) + ftLastModified.dwLowDateTime;
				*lastModifiedTime = (m - 116444736000000000) / 10000000;
			}
		}
		
		DWORD high;
		DWORD low = ::GetFileSize(hFile, &high);
		
		
#ifndef _WIN64
		if (high != 0) {
			fprintf(stderr, "File too large\n");
			fclose(fp);
			return nullptr;
		}
		length = low;
#else
		length = ((size_t)high << 32) | low;
#endif
	} else {
		fprintf(stderr, "Error on getting handle of file\n");
	}

#else
	if (write) {
		if (!MakeDirectoryForFile(path)) {
			return nullptr;
		}
	}

	FILE* fp = fopen(path.c_str(), write ? "wb" : "rb");
	if (fp != nullptr) {
		struct stat buf;
		int fd = fileno(fp);
		fstat(fd, &buf);
		if ((buf.st_mode & S_IFDIR) == S_IFDIR) {
			fclose(fp);
			return nullptr;
		}
		
		if (lastModifiedTime != nullptr) {
			*lastModifiedTime = buf.st_mtime;
		}

		length = buf.st_size;
	} else {
		return nullptr;
	}
#endif

	return new FileStream(fp, std::move(path));
}


void ZArchiveDirent::Query(const String& uri, const TWrapper<void, bool, const String&>& wrapper) const {
	if (FilterSandBox(uri)) {
		return;
	}
	String path = root + uri;

#if defined(_WIN32) || defined(WIN32)
	String searchPath = path + "/*.*";
	String wpath = Utf8ToSystem(searchPath);
	WIN32_FIND_DATAW findData;
	HANDLE hFind = ::FindFirstFileW((WCHAR*)wpath.data(), &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		BOOL hasNext = TRUE;
		do {
			if (findData.cFileName[0] != L'.') {
				String file = SystemToUtf8(String((const char*)findData.cFileName, wcslen(findData.cFileName) * 2));
				wrapper(!!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), file);
			}
		} while (::FindNextFileW(hFind, &findData));
		::FindClose(hFind);
	}

#else
	String wpath = Utf8ToSystem(path);
	DIR* dir = opendir(wpath.c_str());

	if (dir != nullptr) {
		dirent* dp;
		while ((dp = readdir(dir)) != nullptr) {
			if (dp->d_name[0] == '.') continue;
			// convert to locale
			String name = SystemToUtf8(String(dp->d_name));
			wrapper(dp->d_type == DT_DIR, name);
		}

		closedir(dir);
	}
#endif
}

bool ZArchiveDirent::Delete(const String& uri) {
	String path = root + uri;
#if defined(_WIN32) || defined(WIN32)
	String wpath = Utf8ToSystem(path);
	return ::DeleteFileW((WCHAR*)wpath.c_str()) != 0;
#else
	return remove(path.c_str()) == 0;
#endif
}
