#ifndef __File_h__
#define __File_h__

#include "Exception.h"
#include "POSIXErrno.h"
#include <string>

#if _DEBUG_FILE // Debug
inline FILE *printResult(FILE *t) {
  printf("\t RESULT: %08x\n", t);
  return t;
}
inline off_t printResult(off_t t) {
  printf("\t RESULT: %d\n", t);
  return t;
}
#define fopen(p, m) (printf("fopen(%s, %s)\n", p, m), printResult(fopen(p, m)))
#define ftello(f) (printf("ftello(%08x)\n", f), printResult(ftello(f)))
#define fseeko(f, o, w)                                                        \
  (printf("fseeko(%08x, %d, %d)\n", f, o, w), printResult(fseeko(f, o, w)))
#define fread(b, s, n, f)                                                      \
  (printf("fread(%08x, %d, %d, %08x)\n", b, s, n, f),                          \
   printResult(fread(b, s, n, f)))
#define fwrite(b, s, n, f)                                                     \
  (printf("fwrite(%08x, %d, %d, %08x)\n", b, s, n, f),                         \
   printResult(fwrite(b, s, n, f)))
#define fflush(f) (printf("fflush(%08x)\n", f), printResult(fflush(f)))
#define fclose(f) (printf("fclose(%08x)\n", f), printResult(fclose(f)))
#define ferror(f) (printf("ferror(%08x)\n", f), printResult(ferror(f)))
#endif

namespace io {

/// Common file content operations
class File {
public:
  /// Method of reading and writing files, binary or text.
  enum Method { Binary, Text };
  /// How is the file protected, read only, read-write, or write if possible.
  enum Protection {
    ReadOnly,
    ReadWrite,
    /// Try to open the file with write permissions, but do not fail if we
    /// cannot
    WriteIfPossible
  };
  /// Where is the offset relative to, from current position, the start, or the
  /// end.
  enum Relative { FromHere, FromStart, FromEnd };
  /// Read integers as Big Endian, Little Endian, or native.
  enum Endian { BigEndian, LittleEndian, NativeEndian };
  /** Create or open a file at a given path.
        @param path The path to open.
        @param method The type of file, text or binary
        @param protection how to access the file, read only, or read-write
  */
  File(const char *path, Method method, Protection protection);
  /** Create or open a file at a given path.
        @param path The path to open.
        @param method The type of file, text or binary
        @param protection how to access the file, read only, or read-write
  */
  File(const std::string &path, Method method, Protection protection);
  /// destructor.
  virtual ~File();
  /// Gets the size of the file.
  off_t size() const;
  /// Flush any pending writes to the file.
  void flush();
  /// Get the current location in the file.
  off_t location() const;
  /// Is the file writable?
  bool writable() const;
  /** Move to a given location for relative operations.
        @param offset the location to move to
        @param relative Where is offset relative to?
  */
  void moveto(off_t offset, Relative relative = FromStart) const;
  /** Move to a given location for relative operations.
        alias for move(offset, relative)
        @param offset the location to move to
        @param relative Where is offset relative to?
  */
  void move(off_t offset, Relative relative = FromHere) const;
  /** Read a portion of the file.
        @param buffer the buffer to receive the data read from the file.
        @param bufferSize The number of bytes to read from the file into the
     buffer.
        @param offset Where in the file to read the bytes. Defaults to zero
        @param relative Where is offset measured from. Defaults to FromHere
  */
  void read(void *buffer, size_t bufferSize, off_t offset = 0,
            Relative relative = FromHere) const;
  /** Write to a file.
          @param buffer The data to write to the file
          @param bufferSize the number of bytes from buffer to write to the
     file.
          @param offset Where to write the data to the file. Defaults to 0
          @param relative Where is offset relative to. Defaults to FromHere
  */
  void write(const void *buffer, size_t bufferSize, off_t offset = 0,
             Relative relative = FromHere);
  /** Read a portion of the file.
        @param buffer the buffer to receive the data read from the file.
        @param bufferSize The number of bytes to read from the file into the
     buffer. Defaults to reading the rest of the file.
        @param offset Where in the file to read the bytes. Defaults to zero
        @param relative Where is offset measured from. Defaults to FromHere
  */
  std::string &read(std::string &buffer,
                    size_t bufferSize = static_cast<size_t>(-1),
                    off_t offset = 0, Relative relative = FromHere) const;
  /** Write to a file.
          @param buffer The data to write to the file
          @param offset Where to write the data to the file. Defaults to 0
          @param relative Where is offset relative to. Defaults to FromHere
  */
  void write(const std::string &buffer, off_t offset = 0,
             Relative relative = FromHere);
  /** Reads a binary integer from the file.
          @param endian The order of bytes in the file
          @param offset Where to write the data to the file. Defaults to 0
          @param relative Where is offset relative to. Defaults to FromHere
          @returns the integer value read from the file.
  */
  template <class Int>
  Int read(Endian endian, off_t offset = 0, Relative relative = FromHere) const;
  /** Writes a binary integer to a file.
        @param number The integer to write to the file
                @param endian The order of bytes in the file
                @param offset Where to write the data to the file. Defaults to 0
                @param relative Where is offset relative to. Defaults to
     FromHere
  */
  template <class Int>
  void write(Int number, Endian endian, off_t offset = 0,
             Relative relative = FromHere);
  /** Read a line of text from the file.
          Note: The line as well as the end-of-line character(s) are returned.
          @param buffer The buffer to receive the line of text
          @param offset Where to write the data to the file. Defaults to 0
          @param relative Where is offset relative to. Defaults to FromHere
          @param bufferSize The size of blocks of data to read at a time.
     Defaults to 4096
  */
  std::string &readline(std::string &buffer, off_t offset = 0,
                        Relative relative = FromHere,
                        size_t bufferSize = 4096) const;

private:
  FILE *_file;    ///< The file we are working with
  bool _readOnly; ///< Is this file read-only?
  /** Given the enum, calculate the value to pass to the file API.
        @param relative the relative enum for how to interpret offsets
        @return the whence value to pass to the underlying file API.
  */
  static int _whence(Relative relative);
  /** Makes sure we are at the requested location of the file.
        @param offset The offset the user is requesting.
        @param relative What offset is relative to.
  */
  void _goto(off_t offset, Relative relative) const;
  /** Ensure endian is either big endian or little endian.
        @param endian the requested endian
        @return either big endian or little endian. Native endian is translated
     to either big or little endian.
  */
  static Endian _actualEndian(Endian endian);
  /** Handle the common operations when opening a file.
        @param path The file path to open.
        @param method The method used to handle the file, text or binary.
        @param protection Is this file read-only or read-write.
        @param readOnly Set to true if we did not open this file with write
     permissions, false otherwise
  */
  static FILE *_open(const char *path, Method method, Protection protection,
                     bool &readOnly);
  File(const File &);            ///< Mark as unusable
  File &operator=(const File &); ///< Mark as unusable
};

} // namespace io

#define kReadOnlyText "r" ///< Temporary macro to make this file easier to read
#define kOpenReadWriteText                                                     \
  "r+" ///< Temporary macro to make this file easier to read
#define kCreateReadWriteText                                                   \
  "w+" ///< Temporary macro to make this file easier to read
#define kReadOnlyBinary                                                        \
  "rb" ///< Temporary macro to make this file easier to read
#define kOpenReadWriteBinary                                                   \
  "r+b" ///< Temporary macro to make this file easier to read
#define kCreateReadWriteBinary                                                 \
  "w+b" ///< Temporary macro to make this file easier to read

namespace io {

inline File::File(const char *path, Method method, Protection protection)
    : _file(NULL), _readOnly(ReadOnly == protection) {
  _file = _open(path, method, protection, _readOnly);
  moveto(0);
}
inline File::File(const std::string &path, Method method, Protection protection)
    : _file(NULL), _readOnly(ReadOnly == protection) {
  _file = _open(path.c_str(), method, protection, _readOnly);
  moveto(0);
}
inline File::~File() {
  if (NULL != _file) {
    fclose(_file);
    _file = NULL;
  }
}
inline off_t File::size() const {
  off_t here = location();
  off_t end;

  moveto(0, FromEnd);
  end = location();
  moveto(here, FromStart);
  return end;
}
inline void File::flush() { ErrnoOnNegative(fflush(_file)); }
inline off_t File::location() const {
  off_t currentPos;

  ErrnoOnNegative(currentPos = ftello(_file));
  return currentPos;
}
inline bool File::writable() const { return (!_readOnly); }
inline void File::moveto(off_t offset, Relative relative) const {
  ErrnoOnNegative(fseeko(_file, offset, _whence(relative)));
}
inline void File::move(off_t offset, Relative relative) const {
  moveto(offset, relative);
}
inline void File::read(void *buffer, size_t bufferSize, off_t offset,
                       Relative relative) const {
  off_t amount;
  int fileError;

  _goto(offset, relative);
  AssertMessageException(location() + static_cast<off_t>(bufferSize) <= size());
  amount = fread(reinterpret_cast<char *>(buffer), 1, bufferSize, _file);
  AssertMessageException(((fileError = ferror(_file)) == 0) ||
                         (fileError == EOF));
  AssertMessageException(amount == static_cast<off_t>(bufferSize));
}
inline void File::write(const void *buffer, size_t bufferSize, off_t offset,
                        Relative relative) {
  off_t amount;

  AssertMessageException(!_readOnly);
  _goto(offset, relative);
  amount = fwrite(reinterpret_cast<const char *>(buffer), 1, bufferSize, _file);
  AssertMessageException(ferror(_file) == 0);
  AssertMessageException(amount == static_cast<off_t>(bufferSize));
}
inline std::string &File::read(std::string &buffer, size_t bufferSize,
                               off_t offset, Relative relative) const {
  if (static_cast<size_t>(-1) == bufferSize) {
    bufferSize = size() - offset;
  }
  buffer.assign(bufferSize, '\0');
  read(const_cast<char *>(buffer.data()), bufferSize, offset, relative);
  return buffer;
}
inline void File::write(const std::string &buffer, off_t offset,
                        Relative relative) {
  write(buffer.data(), buffer.size(), offset, relative);
}
template <class Int>
inline Int File::read(Endian endian, off_t offset, Relative relative) const {
  uint8_t buffer[sizeof(Int)];
  Int value = 0;

  read(buffer, sizeof(buffer), offset, relative);
  endian = _actualEndian(endian);
  for (unsigned int byte = 0; (byte < sizeof(buffer)); ++byte) {
    Int byteAsInt(buffer[byte]);
    int shiftAmount;

    if (BigEndian == endian) {
      shiftAmount = 8 * (sizeof(buffer) - byte - 1);
    } else {
      shiftAmount = 8 * byte;
    }
    if (shiftAmount != 0) {
      value |= (byteAsInt << shiftAmount);
    } else {
      value |= byteAsInt;
    }
  }
  return value;
}
template <class Int>
inline void File::write(Int number, Endian endian, off_t offset,
                        Relative relative) {
  uint8_t buffer[sizeof(Int)];

  endian = _actualEndian(endian);
  for (unsigned int byte = 0; (byte < sizeof(buffer)); ++byte) {
    int shiftAmount;

    if (BigEndian == endian) {
      shiftAmount = 8 * (sizeof(buffer) - byte - 1);
    } else {
      shiftAmount = 8 * byte;
    }
    if (shiftAmount != 0) {
      buffer[byte] = 0xFF & (number >> shiftAmount);
    } else {
      buffer[byte] = 0xFF & number;
    }
  }
  write(buffer, sizeof(buffer), offset, relative);
}
/**
        @todo improve performance by resizing buffer and using read(void*) to
   read directly into appending buffer
*/
inline std::string &File::readline(std::string &buffer, off_t offset,
                                   Relative relative, size_t bufferSize) const {
  std::string partial;
  std::string::size_type lf, cr, eol;
  off_t left;
  bool foundCR = false, foundLF = false;

  buffer.clear();
  _goto(offset, relative);
  left = size() - location();
  while (!foundCR && !foundLF && (left > 0)) {
    bufferSize = bufferSize > static_cast<size_t>(left)
                     ? static_cast<size_t>(left)
                     : bufferSize;
    read(partial, bufferSize, 0, FromHere);
    left -= partial.size();
    cr = partial.find('\r');
    foundCR = (cr != std::string::npos);
    if (foundCR && (cr == partial.size() - 1) && (left > 0)) {
      char character;

      read(&character, 1, 0, FromHere);
      partial.append(1, character);
      left -= 1;
    }
    lf = partial.find('\n');
    foundLF = (lf != std::string::npos);
    eol = std::string::npos;
    if (foundCR && foundLF) {
      eol = ((cr + 1 == lf) || (lf < cr)) ? lf : cr;
    } else if (foundLF || foundCR) {
      eol = foundLF ? lf : cr;
    }
    if (eol != std::string::npos) {
      const off_t rewindTo = (partial.size() - eol - 1);

      _goto(-1 * rewindTo, FromHere);
      partial.erase(eol + 1);
    }
    buffer.append(partial);
  }
  return buffer;
}
inline int File::_whence(Relative relative) {
  if (FromHere == relative) {
    return SEEK_CUR;
  } else if (FromStart == relative) {
    return SEEK_SET;
  }
  AssertMessageException(FromEnd == relative);
  return SEEK_END;
}
inline void File::_goto(off_t offset, Relative relative) const {
  if ((0 != offset) || (relative != FromHere)) {
    moveto(offset, relative);
  }
}
inline File::Endian File::_actualEndian(Endian endian) {
  if (NativeEndian == endian) {
    uint16_t value = 1;

    if (*reinterpret_cast<uint8_t *>(&value) == 1) {
      return LittleEndian;
    }
    return BigEndian; // not possible to test on little endian systems
  }
  return endian;
}
inline FILE *File::_open(const char *path, File::Method method,
                         File::Protection protection, bool &readOnly) {
  FILE *opened = NULL;
  const bool tryWritable =
      ((ReadWrite == protection) | (WriteIfPossible == protection));

  if (tryWritable) {
    opened = fopen(path, Binary == method ? kOpenReadWriteBinary
                                          : kOpenReadWriteText);
  }
  if ((NULL == opened) && tryWritable) {
    opened = fopen(path, Binary == method ? kCreateReadWriteBinary
                                          : kCreateReadWriteText);
  }
  if (ReadWrite == protection) {
    ErrnoOnNULL(opened); // tried to open it as write, but could not
  }
  if (NULL != opened) {
    readOnly = false;
    return opened;
  }
  opened = fopen(path, Binary == method ? kReadOnlyBinary : kReadOnlyText);
  ErrnoOnNULL(opened);
  readOnly = true;
  return opened;
}
} // namespace io

#undef kReadOnlyText          // This macro is only needed in this file
#undef kOpenReadWriteText     // This macro is only needed in this file
#undef kCreateReadWriteText   // This macro is only needed in this file
#undef kReadOnlyBinary        // This macro is only needed in this file
#undef kOpenReadWriteBinary   // This macro is only needed in this file
#undef kCreateReadWriteBinary // This macro is only needed in this file

#endif // __File_h__
