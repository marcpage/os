#ifndef __FileDescriptor_h__
#define __FileDescriptor_h__

#include "POSIXErrno.h"
#include <fcntl.h> // open/close
#include <string>
#include <sys/stat.h> // fstat
#include <unistd.h>   // ftruncate/fsyncs

namespace io {

class FileDescriptor {
public:
  explicit FileDescriptor(const std::string &path, int oflag = O_RDWR | O_CREAT,
                          mode_t mode = 0777);
  explicit FileDescriptor(const int descriptor);
  struct stat &info(struct stat &buffer) const;
  off_t size() const;
  void resize(off_t newSize) const;
  void sync() const;
  operator int() const;
  void close();
  virtual ~FileDescriptor() throw();

private:
  int _descriptor;
  bool _owned;
  FileDescriptor(const FileDescriptor &);            ///< Mark as unusable
  FileDescriptor &operator=(const FileDescriptor &); ///< Mark as unusable
};

} // namespace io

namespace io {

/**
        @todo Test!
*/
inline FileDescriptor::FileDescriptor(const std::string &path, int oflag,
                                      mode_t mode)
    : _descriptor(ErrnoOnNegative(::open(path.c_str(), oflag, mode))),
      _owned(true) {}
/**
        @todo Test!
*/
inline FileDescriptor::FileDescriptor(const int descriptor)
    : _descriptor(descriptor), _owned(false) {}
/**
        @todo Test!
*/
inline struct stat &FileDescriptor::info(struct stat &buffer) const {
  ErrnoOnNegative(::fstat(_descriptor, &buffer));
  return buffer;
}
/**
        @todo Test!
*/
inline off_t FileDescriptor::size() const {
  struct stat buffer;

  return info(buffer).st_size;
}
/**
        @todo Test!
*/
inline void FileDescriptor::resize(off_t newSize) const {
  ErrnoOnNegative(::ftruncate(_descriptor, newSize));
}
/**
        @todo Test!
*/
inline void FileDescriptor::sync() const {
  ErrnoOnNegative(::fsync(_descriptor));
}
/**
        @todo Test!
*/
inline FileDescriptor::operator int() const { return _descriptor; }
/**
        @todo Test!
*/
inline void FileDescriptor::close() {
  ErrnoOnNegative(::close(_descriptor));
  _descriptor = -1;
}
/**
        @todo Test!
*/
inline FileDescriptor::~FileDescriptor() throw() {
  if (_owned) {
    try {
      close();
    } catch (const std::exception &exception) {
      // ignore
    }
  }
}
} // namespace io

#endif // __FileDescriptor_h__
