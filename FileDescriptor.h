#ifndef __FileDescriptor_h__
#define __FileDescriptor_h__

#include "POSIXErrno.h"
#include <fcntl.h> // open/close
#include <string>
#include <sys/stat.h> // fstat
#include <unistd.h>   // ftruncate/fsyncs

namespace io {

/// System file descriptor
class FileDescriptor {
public:
  /** Get a file descriptor for a path.
          @param path The path to open
          @param oflag open options
          @param mode permissions if file is created
          @todo abstract away system flags
  */
  explicit FileDescriptor(const std::string &path, int oflag = O_RDWR | O_CREAT,
                          mode_t mode = 0777);
  /** Create a file descriptor object from a system file descriptor.
          @param descriptor the system file descriptor
  */
  explicit FileDescriptor(const int descriptor);
  /** Get information about the file.
        @param buffer the stat structure to fill in.
        @return a reference to buffer
  */
  struct stat &info(struct stat &buffer) const;
  /// Get the size of the file.
  off_t size() const;
  /** Set the size of the file.
        @param newSize the new size of the file.
  */
  void resize(off_t newSize) const;
  /// Synchronize the file state in memory with disk.
  void sync() const;
  /// Typecaster operator to system file descriptor.
  operator int() const { return _descriptor; }
  /// Close the file descriptor.
  void close();
  /// Will close the file descriptor if owned (was created from a path, not a
  /// system descriptor).
  virtual ~FileDescriptor() throw();

private:
  int _descriptor; ///< The system file descriptor
  bool _owned;     ///< true == we created this descriptor, we will close in
                   ///< destructor.
  FileDescriptor(const FileDescriptor &);            ///< Mark as unusable
  FileDescriptor &operator=(const FileDescriptor &); ///< Mark as unusable
};

} // namespace io

namespace io {

inline FileDescriptor::FileDescriptor(const std::string &path, int oflag,
                                      mode_t mode)
    : _descriptor(ErrnoOnNegative(::open(path.c_str(), oflag, mode))),
      _owned(true) {}
/**
        @todo Test!
*/
inline FileDescriptor::FileDescriptor(const int descriptor)
    : _descriptor(descriptor), _owned(false) {}
inline struct stat &FileDescriptor::info(struct stat &buffer) const {
  ErrnoOnNegative(::fstat(_descriptor, &buffer));
  return buffer;
}
inline off_t FileDescriptor::size() const {
  struct stat buffer;

  return info(buffer).st_size;
}
inline void FileDescriptor::resize(off_t newSize) const {
  ErrnoOnNegative(::ftruncate(_descriptor, newSize));
}
/**
        @todo Test!
*/
inline void FileDescriptor::sync() const {
  ErrnoOnNegative(::fsync(_descriptor));
}
inline void FileDescriptor::close() {
  ErrnoOnNegative(::close(_descriptor));
  _descriptor = -1;
}
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
