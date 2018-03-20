#ifndef __Path_h__
#define __Path_h__

#include <string>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <os/POSIXErrno.h>

/**
	@todo Document
	@todo Test
*/
namespace io {

	class Path {
		public:
			enum LinkHandling {
				WorkOnLink,
				WorkOnLinkTarget
			};
			Path(const std::string &path="");
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
			void mkdir(unsigned int mode=0777) const;
			void remove() const;
			void rename(const Path &other) const;
			Path readLink() const;
			void symlink(const Path &contents) const;
			Path parent() const;
			std::string name() const;
			Path operator+(const Path &name) const;
			Path operator+(const std::string &name) const;
			Path &operator=(const Path &other);
			operator std::string() const;
		private:
			std::string _path;
			bool _exists(struct stat &info, LinkHandling action) const;
			void _stat(struct stat &info, LinkHandling action) const;
	};

	inline Path::Path(const std::string &path):_path(path) {
		while ( (_path.length() > 1) && (_path[_path.length() - 1] == '/') ) {
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
	inline bool Path::isAbsolute() const {return _path.length() > 0 && _path[0] == '/';}
	inline bool Path::isRelative() const {return !isAbsolute();}
	inline bool Path::isEmpty() const {return _path.length() == 0;}
	inline void Path::unlink() const {
		ErrnoOnNegative(::unlink(_path.c_str()));
	}
	inline void Path::rmdir() const {
		ErrnoOnNegative(::rmdir(_path.c_str()));
	}
	inline void Path::mkdir(unsigned int mode) const {
		ErrnoOnNegative(::mkdir(_path.c_str(), mode));
	}
	inline void Path::remove() const {
		if (isDirectory()) {
			rmdir();
		} else {
			unlink();
		}
	}
	inline void Path::rename(const Path &other) const {
		ErrnoOnNegative(::rename(_path.c_str(), other._path.c_str()));
	}
	inline Path Path::readLink() const {
		ssize_t length;
		std::string	linkPath(PATH_MAX, '\0');

		ErrnoOnNegative(length = ::readlink(_path.c_str(), const_cast<char*>(linkPath.data()), linkPath.length()));
		linkPath.erase(length);
		return linkPath;
	}
	inline void Path::symlink(const Path &contents) const {
		ErrnoOnNegative(::symlink(contents._path.c_str(), _path.c_str()));
	}
	inline Path Path::parent() const {
		std::string::size_type sepPos = _path.rfind('/');

		if (sepPos == std::string::npos) {
			return Path("");
		}
		if (sepPos == 0) {
			return Path("/");
		}
		return _path.substr(0, sepPos);
	}
	inline std::string Path::name() const {
		std::string::size_type sepPos = _path.rfind('/');

		if (sepPos == std::string::npos) {
			return _path;
		}
		return _path.substr(sepPos + 1);
	}
	inline Path Path::operator+(const Path &name) const {
		return Path(_path + "/" + name._path);
	}
	inline Path Path::operator+(const std::string &name) const {
		return Path(_path + "/" + name);
	}
	inline Path &Path::operator=(const Path &other) {
		_path = other._path;
		return *this;
	}
	inline Path::operator std::string() const {
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
	inline void Path::_stat(struct stat &info, LinkHandling action) const {
		if (WorkOnLink == action) {
			ErrnoOnNegative(::lstat(_path.c_str(), &info));
		} else {
			ErrnoOnNegative(::stat(_path.c_str(), &info));
		}

	}
}

#endif // __Path_h__
