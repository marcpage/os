#ifndef __MemoryMappedFile_h__
#define __MemoryMappedFile_h__

#include "FileDescriptor.h"
#include "POSIXErrno.h"
#include <string>
#include <sys/mman.h> // mmap

// TODO: Add integer read and write
namespace io {

/// Memory mapped file for crash-resistance and faster access
class MemoryMappedFile {
public:
  /** Given a file descriptor, map it to a memory address.
          @param file file descriptor to map
          @param size The number of bytes to map into memory. Defaults to 0
     which means the rest of the file.
          @param offset The offset in the file descriptor to map into memory.
     Defaults to the beginning, 0.
          @param protections The access allowed. Defaults to read/write
          @param flags Flags for mapping. Defaults to shared memory.
  */
  explicit MemoryMappedFile(const FileDescriptor &file, size_t size = 0,
                            size_t offset = 0,
                            int protections = PROT_READ | PROT_WRITE,
                            int flags = MAP_SHARED);
  /** Given the path to a file, map it to a memory address.
          @param file The path to the file to path
          @param size The number of bytes to map into memory. Defaults to 0
     which means the rest of the file.
          @param offset The offset in the file descriptor to map into memory.
     Defaults to the beginning, 0.
          @param protections The access allowed. Defaults to read/write
          @param flags Flags for mapping. Defaults to shared memory.
  */
  explicit MemoryMappedFile(const std::string &file, size_t size = 0,
                            size_t offset = 0,
                            int protections = PROT_READ | PROT_WRITE,
                            int flags = MAP_SHARED);
  /// Get the memory address the file is mapped to
  operator void *();
  /// Get the size in bytes of the mapped portion of the file
  size_t size() { return _size; }
  /// Get the address of the file and treat it as a specific data type
  template <class T> T *address();
  /// Get the number of items of the given data type that fit in the space.
  template <class T> size_t count();
  /// Close the reference to the memory mapped file
  void close();
  /// destructor
  virtual ~MemoryMappedFile();

private:
  size_t _size;                               ///< Size of the mapped space
  void *_address;                             ///< address of the mapped file
  MemoryMappedFile(const MemoryMappedFile &); ///< Mark as unusable
  MemoryMappedFile &operator=(const MemoryMappedFile &); ///< Mark as unusable
};

} // namespace io

namespace io {
inline MemoryMappedFile::MemoryMappedFile(const FileDescriptor &file,
                                          size_t size, size_t offset,
                                          int protections, int flags)
    : _size(size > 0 ? size : (file.size() - offset)),
      _address(ErrnoOnNULL(
          ::mmap(nullptr, _size, protections, flags, file, offset))) {}
inline MemoryMappedFile::MemoryMappedFile(const std::string &file, size_t size,
                                          size_t offset, int protections,
                                          int flags)
    : _size(0), _address(nullptr) {
  FileDescriptor fd(file);

  _size = size > 0 ? size : (fd.size() - offset);
  _address =
      ErrnoOnNULL(::mmap(nullptr, _size, protections, flags, fd, offset));
}
inline MemoryMappedFile::operator void *() {
  if (nullptr == _address) {
    ThrowMessageException("Memory Mapped File already closed"); // not tested
  }
  return _address;
}
void MemoryMappedFile::close() {
  if (nullptr != _address) {
    ErrnoOnNegative(::munmap(_address, _size));
  }
  _address = nullptr;
  _size = 0;
}
template <class T> inline T *MemoryMappedFile::address() {
  if (nullptr == _address) {
    ThrowMessageException("Memory Mapped File already closed"); // not tested
  }
  if (sizeof(T) > _size) {
    ThrowMessageException( // not tested
        "Memory Mapped File is not big enough for requested type");
  }
  return reinterpret_cast<T *>(_address);
}
/**
        @todo Test!
*/
template <class T> inline size_t MemoryMappedFile::count() {
  if (nullptr == _address) {
    ThrowMessageException("Memory Mapped File already closed"); // not tested
  }
  return _size / sizeof(T);
}

inline MemoryMappedFile::~MemoryMappedFile() {
  try {
    close();
  } catch (const std::exception &) {
    // cannot handle error in exception
  }
}

} // namespace io

#endif // __MemoryMappedFile_h__
