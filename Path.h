#ifndef __Path_h__
#define __Path_h__

#include "os/DateTime.h"
#include "os/File.h"
#include "os/POSIXErrno.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

/**
        @todo chmod
*/
namespace io {

/// Represents a path on disk and operations you can perform on that path.
class Path {
public:
  /// Should we operator on the link, or what the link points to?
  enum LinkHandling { WorkOnLink, WorkOnLinkTarget };
  /// List everything under the directory or just at the root?
  enum Depth { RecursiveListing, FlatListing };
  /// Do we want the name of directory contents or full path?
  enum HavePath { NameOnly, PathAndName };
  typedef std::string String;             ///< The type of string used
  typedef std::vector<String> StringList; ///< string list
  /** Determines if a string ends with a path separator.
        @param text the text to check
        @return true if the platform path separator is at the end of the text.
  */
  static bool endsWithPathSeparator(const String &text);
  /// Get the current working directory.
  static Path working();
  /// Construct a path from a string.
  explicit Path(const char *path);
  /// Construct a path from a string. defaults to empty path.s
  Path(const String &path = "");
  /// copy constructor.
  Path(const Path &other);
  /// destructor.
  ~Path() {}
  /** Checks to see if there is something on disk at the path.
        @param action defaults to WorkOnLinkTarget, which will return true if
     the link points to an item.
        @return true if the item exists
  */
  bool exists(LinkHandling action = WorkOnLinkTarget) const;
  /** Does the path represent a directory.
        @param action defaults to WorkOnLinkTarget, which will return true if
     the link points to a directory.
  */
  bool isDirectory(LinkHandling action = WorkOnLinkTarget) const;
  /** Does the path represent a file.
        @param action defaults to WorkOnLinkTarget, which will return true if
     the link points to a file.
  */
  bool isFile(LinkHandling action = WorkOnLinkTarget) const;
  /// Does this path represent a link on disk.
  bool isLink() const;
  /// Is this path a relative path
  bool isRelative() const;
  /// Is this path an absolute path
  bool isAbsolute() const;
  /// Is this an empty path
  bool isEmpty() const;
  /// Delete file.
  void unlink() const;
  /// Delete an empty directory.
  void rmdir() const;
  /// Delete the path item, regardless of file or directory or contents.
  void remove() const;
  /** Create a directory.
        @param mode defaults to 0777 which is means read/write/execute
     permissions.
  */
  void mkdir(unsigned int mode = 0777) const;
  /** Create a directory and all directories leading up to it.
        @param mode defaults to 0777 which is means read/write/execute
     permissions.
        @return a reference to this path.
  */
  const Path &mkdirs(unsigned int mode = 0777) const;
  /// Rename an item or move it from one path to another.
  void rename(const Path &other) const;
  // TODO add copyAttributesTo(const path &other) const;
  /// Copies the contents of a file from one path to another.
  const Path &copyContentsTo(const Path &other) const;
  /// Get the location a link points to.
  Path readLink() const;
  /// Returns a relative path that if added to other would result in this path.
  Path relativeTo(const Path &other) const;
  /// Returns a relative path that if added to other would result in this path.
  Path relativeTo(const std::string &other) const {
    return relativeTo(Path(other));
  }
  /** Convert a path from relative to absolute.
        @param from The path that this path is relative to. Defaults to the
     current working dirctory.
        @return An absolute path.
  */
  Path absolute(const Path &from = working()) const;
  /// Creates a symlink to contents
  void symlink(const Path &contents) const;
  /// Get the parent directory.
  Path parent() const;
  /// Get the filename
  String name() const;
  /// Get the extension, excluding the dot (.) of the filename, or empty string
  String extension() const;
  /// Get the name of the file without the extension
  String basename() const;
  /// Remove symlinks, extra separators, and relative paths.
  Path canonical() const;
  /** Create a path that does not exist on disk.
        The filename core is eight random characters.
        @param prefix The prefix for the filename. Defaults to no prefix
        @param suffix The suffix to append onto the filename. Defaults to no
     suffix. This could include an extension.
  */
  Path uniqueName(const std::string &prefix = "",
                  const std::string &suffix = "") const;
  /** Write data to a file.
          @param contents the contents of the file.
          @param method The method, text or binary, to use. Defaults to
     io::File::Text
  */
  void write(const std::string &contents,
             io::File::Method method = io::File::Text) const;
  /** Get the contents of a file.
         @param method the method to use to read the file. Defaults to
     io::File::Text
         @param offset The offset to read from the file. Defaults to the
     beginning, 0.
         @param size The number of bytes to read from the file. Defaults to all
     remaining.
         @return The contents of the file.
  */
  String contents(io::File::Method method = io::File::Text, off_t offset = 0,
                  size_t size = static_cast<size_t>(-1)) const {
    String buffer;
    return contents(buffer, method, offset, size);
  }
  /** Get the contents of a file.
         @param buffer The buffer to fill with the contents.
         @param method the method to use to read the file. Defaults to
     io::File::Text
         @param offset The offset to read from the file. Defaults to the
     beginning, 0.
         @param size The number of bytes to read from the file. Defaults to all
     remaining.
         @return A reference to buffer.
  */
  String &contents(String &buffer, io::File::Method method = io::File::Text,
                   off_t offset = 0,
                   size_t size = static_cast<size_t>(-1)) const;
  /** Get the contents of a directory.
          Does not include '.' or '..'
          @param havePath NameOnly just return the names of the items,
     PathAndName return the full path to every item.
          @param recursive RecursiveListing if subdirectories are to be
     searched, FlatListing if just the contents of the directory.
          @return A list of the strings that represent the contents of the
     directory.
  */
  StringList list(HavePath havePath, Depth recursive = FlatListing) const;
  /** Get the contents of a directory.
          Does not include '.' or '..'
          @param havePath NameOnly just return the names of the items,
     PathAndName return the full path to every item.
          @param directoryListing a list to add the contents strings to.
          @param recursive RecursiveListing if subdirectories are to be
     searched, FlatListing if just the contents of the directory.
          @return A reference to directoryListing
  */
  StringList &list(HavePath havePath, StringList &directoryListing,
                   Depth recursive = FlatListing) const;
  /** Concatenate a relative path onto this path.
          @param name a relative path to append
          @return a path that represents this path plus a relative path
  */
  Path operator+(const Path &name) const;
  /** Concatenate a relative path onto this path.
          @param name a relative path to append
          @return a path that represents this path plus a relative path
  */
  Path operator+(const std::string &name) const {
    return *this + io::Path(name);
  }
  /// assignment operator.
  Path &operator=(const Path &other);
  /// Compare canonical version of two paths.
  bool operator==(const Path &other);
  /// Compare canonical version of two paths.
  bool operator!=(const Path &other) { return !(*this == other); }
  /// typecast this path to a string.
  operator String() const;
  /** Get the device id.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The identifier of the device the file is on.
  */
  dev_t device(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the inode.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The identifier of the file contents.
  */
  ino_t inode(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the permissions.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The permissions of the file.
  */
  mode_t permissions(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the number of inode links.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The number of inode links of the file contents.
  */
  nlink_t links(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the id of the owner.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The id of the owner of the file.
  */
  uid_t userId(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the id of the owning group.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The id of the owning group of the file.
  */
  gid_t groupId(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the time when the file was last accessed.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The time when the file was last accessed.
  */
  dt::DateTime lastAccess(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the time when the file was last modified.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The time when the file was last modified.
  */
  dt::DateTime lastModification(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the time when the file contents or attributes were last modified.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The time when the file contents or attributes were last
     modified.
  */
  dt::DateTime lastStatusChange(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the time when the file was created.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The time when the file was created.
  */
  dt::DateTime created(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the size of the file.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The size of the file.
  */
  off_t size(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the number of blocks in the file.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The number of blocks in the file.
  */
  off_t blocks(LinkHandling action = WorkOnLinkTarget) const;
  /** Get the size of blocks used for the file.
        @param action defaults to WorkOnLinkTarget, which will return
     information of the file a link references.
        @return The size of blocks used for the file.
  */
  off_t blockSize(LinkHandling action = WorkOnLinkTarget) const;

private:
  String _path; ///< The path this object represents
  /// Get the stats on a file and determine if it exists.
  bool _exists(struct stat &info, LinkHandling action) const;
  /// List the contents of a directory
  StringList &_list(HavePath havePath, StringList &directoryListing,
                    Depth recursive) const;
  /// Get the stats on a file
  struct stat &_stat(struct stat &info, LinkHandling action) const;
  /// Get the platform-specific separator between path elements
  static const char *_separator() { return "/"; }
};

inline bool Path::endsWithPathSeparator(const String &text) {
  static const auto separator = _separator()[0];

  return (text.length() > 1) && (text[text.length() - 1] == separator);
}

inline Path Path::working() {
  char buffer[MAXPATHLEN];

  return Path(ErrnoOnNULL(getcwd(buffer, sizeof(buffer))));
}

inline Path::Path(const char *path) : Path(String(path)) {}
inline Path::Path(const String &path) : _path(path) {
  while ((_path.length() > 1) && endsWithPathSeparator(_path)) {
    _path.erase(_path.length() - 1);
  }
}
inline Path::Path(const Path &other) : _path(other._path) {}
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
inline bool Path::isAbsolute() const {
  return _path.length() > 0 && _path[0] == _separator()[0];
}
inline bool Path::isRelative() const { return !isAbsolute(); }
inline bool Path::isEmpty() const { return _path.length() == 0; }
inline void Path::unlink() const { ErrnoOnNegative(::unlink(_path.c_str())); }
inline void Path::rmdir() const { ErrnoOnNegative(::rmdir(_path.c_str())); }
inline void Path::remove() const {
  if (isDirectory(WorkOnLink)) {
    StringList contents;

    list(PathAndName, contents);
    for (auto i : contents) {
      Path(i).remove();
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
/// @todo Test
const Path &Path::copyContentsTo(const Path &other) const {
  io::File src(_path, io::File::Binary, io::File::ReadOnly);
  io::File dst(other._path, io::File::Binary, io::File::ReadWrite);
  auto bytesLeft = size();
  std::string buffer;
  static const decltype(bytesLeft) blockSize = 1024 * 1024;

  while (bytesLeft > 0) {
    src.read(buffer, std::min(blockSize, bytesLeft));
    dst.write(buffer);
    bytesLeft -= buffer.size();
  }
  return other;
}
inline Path Path::readLink() const {
  ssize_t length;
  String linkPath(PATH_MAX, '\0');

  ErrnoOnNegative(length = ::readlink(_path.c_str(),
                                      const_cast<char *>(linkPath.data()),
                                      linkPath.length()));
  linkPath.erase(length);
  return linkPath;
}
inline Path Path::relativeTo(const Path &other) const {
  if (_path == other._path) {
    return Path("");
  }

  if (!isAbsolute()) {
    ThrowMessageException(String("Path is not absolute: '") + String(*this) +
                          "'");
  }

  if (!other.isAbsolute()) {
    ThrowMessageException(String("Path is not absolute: '") + String(other) +
                          "'");
  }

  auto matchSeparator = String::npos;
  static const auto separator = _separator()[0];
  const auto me = other._path + separator;
  const auto you = _path + separator;
  Path result;

  if ((you.length() > me.length()) && (you[me.length() - 1] == separator) &&
      (you.substr(0, me.length()) == me)) {
    return you.substr(me.length() + (you[me.length()] == separator ? 1 : 0));
  }

  for (String::size_type position = 0;
       position < std::min(me.length(), you.length()) &&
       (me[position] == you[position]);
       ++position) {
    if (separator == me[position]) {
      matchSeparator = position;
    }
  }

  for (auto position = matchSeparator + 1; position < me.length(); ++position) {
    if (separator == me[position]) {
      result = result + String("..");
    }
  }

  return result + you.substr(matchSeparator + 1);
}
inline Path Path::absolute(const Path &from) const {
  if (isAbsolute()) {
    return *this;
  }
  return (from + *this).canonical();
}
inline void Path::symlink(const Path &contents) const {
  ErrnoOnNegative(::symlink(contents._path.c_str(), _path.c_str()));
}
inline Path Path::parent() const {
  auto sepPos = _path.rfind(_separator()[0]);

  if (sepPos == String::npos) {
    return Path("");
  }
  if (sepPos == 0) {
    return Path(_separator()); // root
  }
  return _path.substr(0, sepPos);
}
inline Path::String Path::name() const {
  auto sepPos = _path.rfind(_separator()[0]);

  if (sepPos == String::npos) {
    return _path;
  }
  return _path.substr(sepPos + 1);
}
inline Path::String Path::extension() const {
  auto sepPos = _path.rfind('.');

  if (sepPos == String::npos) {
    return "";
  }
  return _path.substr(sepPos + 1);
}
inline Path::String Path::basename() const {
  auto fullname = name();
  auto sepPos = fullname.rfind('.');

  if (sepPos == String::npos) {
    return fullname;
  }
  return fullname.substr(0, sepPos);
}
inline Path Path::canonical() const {
  String buffer(PATH_MAX, '\0');

  ErrnoOnNULL(
      ::realpath(String(*this).c_str(), const_cast<char *>(buffer.data())));
  buffer.erase(::strlen(buffer.c_str()));
  return buffer;
}
inline Path Path::uniqueName(const std::string &prefix,
                             const std::string &suffix) const {
  static const char *const characters =
      "abcdefghijklmnopqrstuvwxyz0123456789_-+=~";
  static const int characterCount = strlen(characters);

  while (true) {
    std::string name;

    for (int i = 0; i < 8; ++i) {
      name += characters[rand() % characterCount];
    }

    Path result = (*this) + (prefix + name + suffix);

    if (!result.exists()) {
      return result;
    }
  }
}
inline Path::String &Path::contents(String &buffer, io::File::Method method,
                                    off_t offset, size_t size) const {
  io::File(_path, method, io::File::ReadOnly).read(buffer, size, offset);
  return buffer;
}
inline void Path::write(const std::string &contents,
                        io::File::Method method) const {
  io::File(_path, method, io::File::ReadWrite).write(contents);
}
inline Path::StringList Path::list(HavePath havePath, Depth recursive) const {
  StringList directoryListing;

  return list(havePath, directoryListing, recursive);
}
inline Path::StringList &Path::list(HavePath havePath,
                                    StringList &directoryListing,
                                    Depth recursive) const {

  return _list(havePath, directoryListing, recursive);
}
inline Path::StringList &Path::_list(HavePath havePath,
                                     StringList &directoryListing,
                                     Depth recursive) const {
  DIR *dp;
  StringList directories;

  ErrnoOnNULL(dp = ::opendir(String(*this).c_str()));

  try {
    bool keepListing = true;

    do {
      try {
        struct dirent *ep;
        ErrnoOnNULL(ep = ::readdir(dp));

        if (NULL != ep) {
#if defined(__APPLE__)
          const String name = String(ep->d_name, 0, ep->d_namlen);
#else
          const String name = String(ep->d_name); //, 0, ep->d_namlen);
#endif
          const bool isDirectory = DT_DIR == ep->d_type;

          if ((name != ".") && (name != "..")) {
            directoryListing.push_back(
                (havePath == NameOnly ? String()
                                      : (String(*this) + _separator())) +
                name + (isDirectory ? String(_separator()) : String()));
            if ((RecursiveListing == recursive) && isDirectory) {
              directories.push_back(String(*this) + _separator() + name);
            }
          }
        } else {
          keepListing = false; // tested in libernet tests
        }
      } catch (const posix::err::ESRCH_Errno &) {
        keepListing = false; // not tested: sometimes we get spurious ESRCH No
                             // such process after listing a directory
      } catch (const posix::err::EINTR_Errno &) { // not covered by tests
        keepListing = false; // not tested: sometimes we get spurious EINTR when
                             // there is an empty directory
      }
    } while (keepListing);
  } catch (const posix::err::ENOENT_Errno &) {
  } catch (const std::exception &) {
    ErrnoOnNegative(::closedir(dp)); // not covered by tests
    throw;                           // not covered by tests
  }

  ErrnoOnNegative(::closedir(dp));

  for (auto directory : directories) {
    Path(directory)._list(havePath, directoryListing, recursive);
  }

  return directoryListing;
}
inline Path Path::operator+(const Path &name) const {
  return Path((_path.length() > 0 ? _path + _separator() : String()) +
              name._path);
}
inline Path &Path::operator=(const Path &other) {
  _path = other._path;
  return *this;
}
inline bool Path::operator==(const Path &other) {
  try {
    return canonical()._path == other.canonical()._path;
  } catch (const posix::err::ENOENT_Errno &) {
    return _path == other._path;
  }
}
inline Path::operator Path::String() const { return _path; }
inline bool Path::_exists(struct stat &info, LinkHandling action) const {
  try {
    _stat(info, action);
  } catch (const posix::err::ENOENT_Errno &) {
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

#if defined(__APPLE__)
  return dt::DateTime(_stat(info, action).st_atimespec);
#else
  return dt::DateTime(_stat(info, action).st_atime);
#endif
}
inline dt::DateTime Path::lastModification(LinkHandling action) const {
  struct stat info;

#if defined(__APPLE__)
  return dt::DateTime(_stat(info, action).st_mtimespec);
#else
  return dt::DateTime(_stat(info, action).st_mtime);
#endif
}
inline dt::DateTime Path::lastStatusChange(LinkHandling action) const {
  struct stat info;

#if defined(__APPLE__)
  return dt::DateTime(_stat(info, action).st_ctimespec);
#else
  return dt::DateTime(_stat(info, action).st_ctime);
#endif
}
inline dt::DateTime Path::created(LinkHandling action) const {
  struct stat info;

#if defined(__APPLE__)
  return dt::DateTime(_stat(info, action).st_birthtimespec);
#else
  return dt::DateTime(_stat(info, action).st_mtime);
#endif
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

} // namespace io

#endif // __Path_h__
