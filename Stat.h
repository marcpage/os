#ifndef __Stat_h__
#define __Stat_h__

#include "MessageException.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <vector>
#include <dirent.h>

namespace io {
	class Stat {
		public:
			enum ResolveSymLinks {
				LookAtSymlink,
				LookAtLinkTarget
			};
			enum TimeZone {
				GMT,
				Local
			};
			enum ListingMode {
				AllFiles,
				IgnoreDotted
			};
			typedef std::vector<std::string>	StringList;
			Stat(const char *pathParam= NULL, ResolveSymLinks symlink= LookAtLinkTarget);
			Stat(const Stat &other);
			~Stat();
			Stat &operator=(const Stat &other);
			const std::string &path() const;
			uid_t uid() const;
			gid_t gid() const;
			off_t size() const;
			blksize_t blocksize() const;
			blkcnt_t blocks() const;
			time_t access() const;
			time_t modified() const;
			time_t status() const;
			mode_t mode() const;
			std::string &readlink(std::string &linkParam) const;
			std::string &user(std::string &name) const;
			std::string &group(std::string &name) const;
			std::string &access(std::string &time, TimeZone zone= GMT) const;
			std::string &modified(std::string &time, TimeZone zone= GMT) const;
			std::string &status(std::string &time, TimeZone zone= GMT) const;
			off_t allocated() const;
			bool readable() const;
			bool readonly() const;
			bool readwrite() const;
			bool executable() const;
			bool writable() const;
			bool file() const;
			bool directory() const;
			bool link() const;
			void list(StringList *files, StringList *directories= NULL, ListingMode mode= IgnoreDotted) const;
		private:
			std::string	_path;
			struct stat	_stat;
			static std::string &_stringify(time_t seconds, std::string &time, TimeZone zone);
			static int _allFiles(struct dirent *entry);
			static int _allDirectories(struct dirent *entry);
			static int _all(struct dirent *entry);
			static int _visibleFiles(struct dirent *entry);
			static int _visibleDirectories(struct dirent *entry);
			static int _visible(struct dirent *entry);
			static bool _specialDirectory(const struct dirent *entry);
			static bool _hidden(const struct dirent *entry);
	};

	inline Stat::Stat(const char *pathParam, ResolveSymLinks symlink)
		:_path(NULL == pathParam ? "" : pathParam), _stat() {
		if(NULL == pathParam) {
			memset(&_stat, 0, sizeof(_stat));
		} else if(LookAtSymlink == symlink) {
			AssertMessageException(0 == lstat(pathParam, &_stat));
		} else {
			AssertMessageException(0 == stat(pathParam, &_stat));
		}
	}
	inline Stat::Stat(const Stat &other)
		:_path(other._path), _stat(other._stat) {
	}
	inline Stat::~Stat() {
	}
	inline Stat &Stat::operator=(const Stat &other) {
		_path= other._path;
		memcpy(&_stat, &other._stat, sizeof(_stat));
		return *this;
	}
	inline const std::string &Stat::path() const {
		return _path;
	}
	inline uid_t Stat::uid() const {
		return _stat.st_uid;
	}
	inline gid_t Stat::gid() const {
		return _stat.st_gid;
	}
	inline off_t Stat::size() const {
		return _stat.st_size;
	}
	inline blksize_t Stat::blocksize() const {
		return _stat.st_blksize;
	}
	inline blkcnt_t Stat::blocks() const {
		return _stat.st_blocks;
	}
	inline time_t Stat::access() const {
		return _stat.st_atime;
	}
	inline time_t Stat::modified() const {
		return _stat.st_mtime;
	}
	inline time_t Stat::status() const {
		return _stat.st_ctime;
	}
	inline mode_t Stat::mode() const {
		return _stat.st_mode;
	}
	inline std::string &Stat::readlink(std::string &linkParam) const {
		ssize_t		length;
		
		linkParam.clear();
		do	{
			linkParam.append(256, '\0');
			errnoAssertPositiveMessageException(length= ::readlink(_path.c_str(),
																	const_cast<char*>(linkParam.data()),
																	linkParam.size()
																));
		} while(length >= static_cast<int>(linkParam.size()));
		linkParam.resize(length);
		return linkParam;
	}
	inline std::string &Stat::user(std::string &name) const {
		struct passwd	*info;
		
		errnoNULLAssertMessageException(info= getpwuid(uid()));
		name.assign(info->pw_name);
		return name;
	}
	inline std::string &Stat::group(std::string &name) const {
		struct group	*info;
		
		errnoNULLAssertMessageException(info= getgrgid(gid()));
		name.assign(info->gr_name);
		return name;
	}
	std::string &Stat::access(std::string &time, TimeZone zone) const {
		return _stringify(access(), time, zone);
	}
	std::string &Stat::modified(std::string &time, TimeZone zone) const {
		return _stringify(modified(), time, zone);
	}
	std::string &Stat::status(std::string &time, TimeZone zone) const {
		return _stringify(status(), time, zone);
	}
	inline off_t Stat::allocated() const {
		return 512 * blocks();
	}
	inline bool Stat::readable() const {
		return ((mode() & (S_IRUSR | S_IRGRP | S_IROTH)) != 0);
	}
	inline bool Stat::readonly() const {
		return readable() &&  !writable();
	}
	inline bool Stat::readwrite() const {
		return readable() && writable();
	}
	inline bool Stat::executable() const {
		return ((mode() & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0);
	}
	inline bool Stat::writable() const {
		return ((mode() & (S_IWUSR | S_IWGRP | S_IWOTH)) != 0);
	}
	inline bool Stat::file() const {
		return S_ISREG(_stat.st_mode);
	}
	inline bool Stat::directory() const {
		return S_ISDIR(_stat.st_mode);
	}
	inline bool Stat::link() const {
		return S_ISLNK(_stat.st_mode);
	}
	inline void Stat::list(StringList *files, StringList *directories, ListingMode modeParam) const {
		typedef int(*Filter)(struct dirent*);
		Filter			toUse;
		int				count;
		struct dirent	**entries;
		const char		*exception= NULL;
		
		AssertMessageException(directory());
		AssertMessageException( (NULL != files) || (NULL != directories) );
		if(NULL == files) {
			if(AllFiles == modeParam) {
				toUse= _allDirectories;
			} else {
				toUse= _visibleDirectories;
			}
		} else if(NULL == directories) {
			if(AllFiles == modeParam) {
				toUse= _allFiles;
			} else {
				toUse= _visibleFiles;
			}
		} else {
			if(AllFiles == modeParam) {
				toUse= _all;
			} else {
				toUse= _visible;
			}
		}
		if(NULL != files) {
			files->clear();
		}
		if(NULL != directories) {
			directories->clear();
		}
		errnoAssertPositiveMessageException( count= scandir(_path.c_str(), &entries, toUse, alphasort) );
		for(int index= 0; index < count; ++index) {
			if( (NULL != files) && (entries[index]->d_type == DT_REG) ) {
				files->push_back(std::string(entries[index]->d_name, entries[index]->d_namlen));
			} else if( (NULL != directories) && (entries[index]->d_type == DT_DIR) ) {
				directories->push_back(std::string(entries[index]->d_name, entries[index]->d_namlen));
			} else {
				exception= "We got something we did not expect";
			}
			free(entries[index]);
		}
		free(entries);
		if(NULL != exception) {
			ThrowMessageException(exception);
		}
	}
	/** 
		Format: YYYY/MM/DD:HH:MM:SS
			in GMT time
	*/
	inline std::string &Stat::_stringify(time_t seconds, std::string &time, TimeZone zone) {
		struct tm	info;
		int			length;
		const int	kNullTerminationCharacter= 1;
		
		if(GMT == zone) {
			gmtime_r(&seconds, &info);
		} else {
			localtime_r(&seconds, &info);
		}
		time.assign(19 + kNullTerminationCharacter, '\0');
		length= snprintf(const_cast<char*>(time.data()), time.size(),
					"%04d/%02d/%02d:%02d:%02d:%02d",
					1900 + info.tm_year, 1 + info.tm_mon, info.tm_mday,
					info.tm_hour, info.tm_min, info.tm_sec);
		AssertMessageException(length == static_cast<int>(time.size() - kNullTerminationCharacter));
		time.resize(length);
		return time;
	}
	inline int Stat::_allFiles(struct dirent *entry) {
		if(_specialDirectory(entry)) {
			return 0;
		}
		return entry->d_type == DT_REG ? 1 : 0;
	}
	inline int Stat::_allDirectories(struct dirent *entry) {
		if(_specialDirectory(entry)) {
			return 0;
		}
		return entry->d_type == DT_DIR ? 1 : 0;
	}
	inline int Stat::_all(struct dirent *entry) {
		if(_specialDirectory(entry)) {
			return 0;
		}
		return (entry->d_type == DT_DIR) || (entry->d_type == DT_REG);
	}
	inline int Stat::_visibleFiles(struct dirent *entry) {
		if(_allFiles(entry) && !_hidden(entry)) {
			return 1;
		}
		return 0;
	}
	inline int Stat::_visibleDirectories(struct dirent *entry) {
		if(_allDirectories(entry) && !_hidden(entry)) {
			return 1;
		}
		return 0;
	}
	inline int Stat::_visible(struct dirent *entry) {
		if(_all(entry) && !_hidden(entry)) {
			return 1;
		}
		return 0;
	}
	inline bool Stat::_specialDirectory(const struct dirent *entry) {
		if( (entry->d_namlen == 1) && (entry->d_name[0] == '.') ) {
			return true;
		}
		if( (entry->d_namlen == 2) && (entry->d_name[0] == '.') && (entry->d_name[1] == '.') ) {
			return true;
		}
		return false;
	}
	inline bool Stat::_hidden(const struct dirent *entry) {
		return (entry->d_namlen >= 1) && (entry->d_name[0] == '.');
	}
}

#endif // __Stat_h__
