#ifndef __ZCompression_h__
#define __ZCompression_h__

#include "Exception.h"
#include <zlib.h>

/** Handle standard zlib return codes and convert errors to exceptions.
        @param code An inline zlib function or return code from a zlib function.
*/
#define zlib_handle_error(code)                                                \
  if (0 != (code))                                                             \
    throw z::Exception(code, __FILE__, __LINE__);                              \
  else                                                                         \
    z::noop()

namespace z {

/// Noop to support zlib_handle_error macro. Does nothing.
inline void noop() {}

/** zlib base exception.
        @todo look to implement different subclass exceptions like errno.
*/
class Exception : public msg::Exception {
public:
  typedef msg::Exception Super; ///< The parent exception class
  /** Converts a zlib return error code into an exception. */
  explicit Exception(int zcode, const char *file = NULL, int line = 0) throw();
  /** Copy constructor. */
  Exception(const Exception &other);
  /** Assignment operator. */
  Exception &operator=(const Exception &other);
  /** Destructor. */
  // cppcheck-suppress missingOverride
  virtual ~Exception() throw();
  /** The message describing the zlib error. */
  const char *what() const throw() override;
  /** The actual error code returned by zlib. */
  int code();

private:
  int _code; ///< The zlib error return code
  /** Gets a string describing the error. */
  static const char *_codestring(int code);
};

/**
        @param zcode the zlib error return code.
        @param file The file in which the error happened
        @param line the line number which threw the exception
*/
inline Exception::Exception(int zcode, const char *file, int line) throw()
    : Super(_codestring(zcode), file, line), _code(zcode) {}
/** @param other The exception to copy. */
inline Exception::Exception(const Exception &other)
    : Super(other), _code(other._code) {}
/** @param other The exception to copy. */
inline Exception &Exception::operator=(const Exception &other) {
  Super::operator=(other);
  _code = other._code;
  return *this;
}
inline Exception::~Exception() throw() {}
/** @return The description of the error. */
inline const char *Exception::what() const throw() { return Super::what(); }
/** @return the zlib error return code. */
inline int Exception::code() { return _code; }
/**
        @param code The zlib error return code.
        @return A string describing the error.
*/
inline const char *Exception::_codestring(int code) {
  switch (code) {
  case Z_OK:
    break;
  case Z_MEM_ERROR:
    return "Out of Memory";
  case Z_BUF_ERROR:
    return "Destination was not big enough";
  case Z_STREAM_ERROR:
    return "level was not valid";
  case Z_DATA_ERROR:
    return "Corrupt compressed data";
  default:
    return "Unknown error code";
  }
  return "";
}

/** Gets the larges size a compressed source size can be.
        This can be used to preallocate buffers before compressing data.
        @param sourceSize The size of the uncompressed data.
        @return The maximum size of the compressed data.
*/
inline size_t maxCompressedSize(size_t sourceSize) {
  return ::compressBound(sourceSize);
}

/** Compresses data.
        @param source Pointer to the data to compress
        @param sourceSize The number of bytes to compress
        @param destination The location to write the compressed data
        @param destinationSize The maximum number of bytes that can be written
   to destination
        @param level The compression level. 0 = none, 9 = maximum, default = 6
        @return The number of bytes written to destination.
        @throws msg::Exception if destinationSize is less than
   maxCompressedSize(sourceSize)
        @throws zlib::Exception if there is a problem compressing
*/
inline size_t compress(const void *source, size_t sourceSize, void *destination,
                       size_t destinationSize, int level = 6) {
  uLong dSize = destinationSize;
  AssertMessageException(destinationSize >= maxCompressedSize(sourceSize));

  zlib_handle_error(::compress2(reinterpret_cast<Bytef *>(destination), &dSize,
                                reinterpret_cast<const Bytef *>(source),
                                static_cast<uLong>(sourceSize), level));
  return dSize;
}

/** Compress data.
        @param source the data to compress.
        @param destination Will receive the compressed data
        @param level The compression level. 0 = none, 9 = maximum, default = 6
        @return a reference to destinations
        @throws zlib::Exception if there is a problem compressing
*/
inline std::string &compress(const std::string &source,
                             std::string &destination, int level = 6) {
  const size_t maxDestination = maxCompressedSize(source.size());

  destination.assign(static_cast<std::string::size_type>(maxDestination), '\0');

  const size_t actualDestination =
      compress(source.data(), source.size(),
               const_cast<char *>(destination.data()), maxDestination, level);

  destination.resize(static_cast<std::string::size_type>(actualDestination));
  return destination;
}

/** Compress data.
        @param source the data to compress.
        @param level The compression level. 0 = none, 9 = maximum, default = 6
        @return The compressed data.
        @throws zlib::Exception if there is a problem compressing
*/
inline std::string compress(const std::string &source, int level = 6) {
  std::string buffer;

  return compress(source, buffer, level);
}

/** Uncompresses data.
        @param source Pointer to the data to uncompress
        @param sourceSize The number of bytes to uncompress
        @param destination The location to write the uncompressed data
        @param destinationSize The maximum number of bytes that can be written
   to destination
        @return The number of bytes written to destination.
        @throws zlib::Exception if there is a problem uncompressing.
                        If destinationSize is too small, the code in the
   exception will be Z_BUF_ERROR
*/
inline size_t uncompress(const void *source, size_t sourceSize,
                         void *destination, size_t destinationSize) {
  zlib_handle_error(
      ::uncompress(reinterpret_cast<Bytef *>(destination),
                   static_cast<uLong *>(&destinationSize),
                   const_cast<Bytef *>(reinterpret_cast<const Bytef *>(source)),
                   static_cast<uLong>(sourceSize)));
  return destinationSize;
}

/** Uncompresses data.
        @todo retry with bigger buffer
        @param source The compressed data
        @param destination Will receive the uncompressed data
        @param maxDestination the maximum number of bytes. default = 512k
        @return a reference to destination
        @throws zlib::Exception if there is a problem uncompressing.
                        If maxDestination is too small, the code in the
   exception will be Z_BUF_ERROR
*/
inline std::string &
uncompress(const std::string &source, std::string &destination,
           std::string::size_type maxDestination = 512 * 1024) {
  destination.assign(maxDestination, '\0');

  const size_t actualDestination =
      uncompress(source.data(), source.size(),
                 const_cast<char *>(destination.data()), destination.size());

  destination.resize(static_cast<std::string::size_type>(actualDestination));
  return destination;
}

/** Uncompresses data.
        @todo retry with bigger buffer
        @param source The compressed data
        @param maxDestination the maximum number of bytes. default = 512k
        @return The uncompressed data.
        @throws zlib::Exception if there is a problem uncompressing.
                        If maxDestination is too small, the code in the
   exception will be Z_BUF_ERROR
*/
inline std::string uncompress(const std::string &source,
                              std::string::size_type maxDestination = 512 *
                                                                      1024) {
  std::string results;

  if (source.size() > maxDestination) {
    maxDestination = 2 * source.size();
  }

  uncompress(source, results, maxDestination);
  return results;
}

#undef zlib_handle_error // clean up global preprocessor namespace

}; // namespace z

#endif // __ZCompression_h__
