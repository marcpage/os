#ifndef __Path_h__
#define __Path_h__

#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <vector>
#include "os/POSIXErrno.h"
#include "os/File.h"
#include "os/DateTime.h"

/**
	@todo Document
	@todo test list
	@todo chmod
*/
namespace io {

	class Path {
		public:
			enum LinkHandling {
				WorkOnLink,
				WorkOnLinkTarget
			};
			enum HavePath {
				NameOnly,
				PathAndName
			};
			typedef std::string String;
			typedef std::vector<String> StringList;
			Path(const String &path="");
			Path(const Path &other);
			~Path() {}
			bool exists(LinkHandling action=WorkOnLinkTarget) const;
			bool isDirectory(LinkHandling action=WorkOnLinkTarget) const;
			bool isFile(LinkHandling action=WorkOnLinkTarget) const;
			bool isLink() const;
			bool isRelative() const;
			bool isAbsolute() const;
			bool isEmpty() const;
			void unlink() const;
			void rmdir() const;
			void remove() const;
			void mkdir(unsigned int mode=0777) const;
			const Path &mkdirs(unsigned int mode=0777) const;
			void rename(const Path &other) const;
			Path readLink() const;
			void symlink(const Path &contents) const;
			Path parent() const;
			String name() const;
			Path canonical() const;
			void write(const std::string &contents, io::File::Method method=io::File::Text) const;
			String contents(io::File::Method method=io::File::Text) const {String buffer; return contents(buffer, method);}
			String &contents(String &buffer, io::File::Method method=io::File::Text) const;
			StringList list(HavePath havePath) const;
			StringList &list(HavePath havePath, StringList &directoryListing) const;
			Path operator+(const Path &name) const;
			Path operator+(const String &name) const;
			Path &operator=(const Path &other);
			operator String() const;
			dev_t device(LinkHandling action=WorkOnLinkTarget) const;
			ino_t inode(LinkHandling action=WorkOnLinkTarget) const;
			mode_t permissions(LinkHandling action=WorkOnLinkTarget) const;
			nlink_t links(LinkHandling action=WorkOnLinkTarget) const;
			uid_t userId(LinkHandling action=WorkOnLinkTarget) const;
			gid_t groupId(LinkHandling action=WorkOnLinkTarget) const;
			dt::DateTime lastAccess(LinkHandling action=WorkOnLinkTarget) const;
			dt::DateTime lastModification(LinkHandling action=WorkOnLinkTarget) const;
			dt::DateTime lastStatusChange(LinkHandling action=WorkOnLinkTarget) const;
			dt::DateTime created(LinkHandling action=WorkOnLinkTarget) const;
			off_t size(LinkHandling action=WorkOnLinkTarget) const;
			off_t blocks(LinkHandling action=WorkOnLinkTarget) const;
			off_t blockSize(LinkHandling action=WorkOnLinkTarget) const;

		private:
			String _path;
			bool _exists(struct stat &info, LinkHandling action) const;
			struct stat &_stat(struct stat &info, LinkHandling action) const;
			const char *_separator() const;
	};

	inline Path::Path(const String &path):_path(path) {
		const char separator = _separator()[0];

		while ( (_path.length() > 1) && (_path[_path.length() - 1] == separator) ) {
			_path.erase(_path.length() - 1);
		}
	}
	inline Path::Path(const Path &other):_path(other._path) {}
	inline bool Path::exists(LinkHandling action) const {
		struct stat info;

		return _exists(info, action);
	}
	inline bool Path::isDirectory(LinkHandling action) const {
		struct stat info;

		if (_exists(info, action)) {
			return S_ISDIR(info.st_mode) ? true : false;
		}
		return false;
	}
	inline bool Path::isFile(LinkHandling action) const {
		struct stat info;

		if (_exists(info, action)) {
			return S_ISREG(info.st_mode) ? true : false;
		}
		return false;
	}
	inline bool Path::isLink() const {
		struct stat info;

		if (_exists(info, WorkOnLink)) {
			return S_ISLNK(info.st_mode) ? true : false;
		}
		return false;
	}
	inline bool Path::isAbsolute() const {return _path.length() > 0 && _path[0] == _separator()[0];}
	inline bool Path::isRelative() const {return !isAbsolute();}
	inline bool Path::isEmpty() const {return _path.length() == 0;}
	inline void Path::unlink() const {
		ErrnoOnNegative(::unlink(_path.c_str()));
	}
	inline void Path::rmdir() const {
		ErrnoOnNegative(::rmdir(_path.c_str()));
	}
	inline void Path::remove() const {
		if (isDirectory(WorkOnLink)) {
			StringList contents;

			list(PathAndName, contents);
			for (StringList::iterator i= contents.begin(); i != contents.end(); ++i) {
				Path(*i).remove();
			}
			rmdir();
		} else {
			unlink();
		}
	}
	inline void Path::mkdir(unsigned int mode) const {
		ErrnoOnNegative(::mkdir(_path.c_str(), mode));
	}
	inline const Path &Path::mkdirs(unsigned int mode) const {
		if (!isEmpty()) {
			if (!isDirectory()) {
				parent().mkdirs(mode);
				mkdir(mode);
			}
		}
		return *this;
	}
	inline void Path::rename(const Path &other) const {
		ErrnoOnNegative(::rename(_path.c_str(), other._path.c_str()));
	}
	inline Path Path::readLink() const {
		ssize_t length;
		String	linkPath(PATH_MAX, '\0');

		ErrnoOnNegative(length = ::readlink(_path.c_str(), const_cast<char*>(linkPath.data()), linkPath.length()));
		linkPath.erase(length);
		return linkPath;
	}
	inline void Path::symlink(const Path &contents) const {
		ErrnoOnNegative(::symlink(contents._path.c_str(), _path.c_str()));
	}
	inline Path Path::parent() const {
		String::size_type sepPos = _path.rfind(_separator()[0]);

		if (sepPos == String::npos) {
			return Path("");
		}
		if (sepPos == 0) {
			return Path(_separator()); // root
		}
		return _path.substr(0, sepPos);
	}
	inline Path::String Path::name() const {
		String::size_type sepPos = _path.rfind(_separator()[0]);

		if (sepPos == String::npos) {
			return _path;
		}
		return _path.substr(sepPos + 1);
	}
	inline Path Path::canonical() const {
		String	buffer(PATH_MAX, '\0');

		ErrnoOnNULL(::realpath(String(*this).c_str(), const_cast<char*>(buffer.data())));
		buffer.erase(::strlen(buffer.c_str()));
		return buffer;
	}
	inline Path::String &Path::contents(String &buffer, io::File::Method method) const {
		io::File(_path, method, io::File::ReadOnly).read(buffer);
		return buffer;
	}
	inline void Path::write(const std::string &contents, io::File::Method method) const {
		io::File(_path, method, io::File::ReadWrite).write(contents);
	}
	inline Path::StringList Path::list(HavePath havePath) const {
		StringList directoryListing;

		return list(havePath, directoryListing);
	}
	inline Path::StringList &Path::list(HavePath havePath, StringList &directoryListing) const {
		DIR 			*dp;
		struct dirent	*ep;

		directoryListing.clear();
  		ErrnoOnNULL(dp= ::opendir(String(*this).c_str()));
  		try {
			do {
				ErrnoOnNULL(ep= ::readdir(dp));
				if (NULL != ep) {
					String name= String(ep->d_name, 0, ep->d_namlen);
					if ( (name != ".") && (name != "..")) {
						directoryListing.push_back((havePath == NameOnly ? String() : (String(*this) + _separator())) + name);
					}
				}
			} while (NULL != ep);
		} catch(const posix::err::ENOENT_Errno &) {
		} catch(const std::exception &) {
			ErrnoOnNegative(::closedir(dp));
			throw;
		}
		ErrnoOnNegative(::closedir(dp));
		return directoryListing;
	}
	inline Path Path::operator+(const Path &name) const {
		return Path(_path + _separator() + name._path);
	}
	inline Path Path::operator+(const String &name) const {
		return Path(_path + _separator() + name);
	}
	inline Path &Path::operator=(const Path &other) {
		_path = other._path;
		return *this;
	}
	inline Path::operator Path::String() const {
		return _path;
	}
	inline bool Path::_exists(struct stat &info, LinkHandling action) const {
		try {
			_stat(info, action);
		} catch(const posix::err::ENOENT_Errno &) {
			return false;
		}
		return true;
	}
	inline struct stat &Path::_stat(struct stat &info, LinkHandling action) const {
		if (WorkOnLink == action) {
			ErrnoOnNegative(::lstat(_path.c_str(), &info));
		} else {
			ErrnoOnNegative(::stat(_path.c_str(), &info));
		}
		return info;
	}
	inline const char *Path::_separator() const {
		return "/";
	}
	inline dev_t Path::device(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_dev;
	}
	inline ino_t Path::inode(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_ino;
	}
	inline mode_t Path::permissions(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_mode;
	}
	inline nlink_t Path::links(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_nlink;
	}
	inline uid_t Path::userId(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_uid;
	}
	inline gid_t Path::groupId(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_gid;
	}
	inline dt::DateTime Path::lastAccess(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_atimespec;
	}
	inline dt::DateTime Path::lastModification(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_mtimespec;
	}
	inline dt::DateTime Path::lastStatusChange(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_ctimespec;
	}
	inline dt::DateTime Path::created(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_birthtimespec;
	}
	inline off_t Path::size(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_size;
	}
	inline off_t Path::blocks(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_blocks;
	}
	inline off_t Path::blockSize(LinkHandling action) const {
		struct stat info;

		return _stat(info, action).st_blksize;
	}

}

#endif // __Path_h__
