#ifndef __POSIXErrno_h__
#define __POSIXErrno_h__

#include "os/Exception.h"
#include <errno.h>
#include <exception>
#include <sstream>
#include <string.h>
#include <string>

/// Throw by errno error code name
#define ErrnoThrowAssert(condition, name)                                      \
  throw posix::err::name##_Errno(#condition, __FILE__, __LINE__)
/// Throw by errno error code name
#define ErrnoThrow(name)                                                       \
  throw posix::err::name##_Errno(#name, __FILE__, __LINE__)
/// Throw by errno error code numeric value
#define ErrnoCodeThrow(errnoCode, message)                                     \
  posix::err::Errno::_throw(errnoCode, message, __FILE__, __LINE__)
/// Throw if errno is not 0
#define ErrnoMessageThrow(message)                                             \
  posix::err::Errno::_throw(errno, message, __FILE__, __LINE__)
/// Wrap around a call that sets errno on negative return value
#define ErrnoOnNegative(call)                                                  \
  posix::err::Errno::_throwOnNegative(call, #call, __FILE__, __LINE__)
/// Wrap around a call that sets errno on NULL return value
#define ErrnoOnNULL(call)                                                      \
  posix::err::Errno::_throwOnNull(call, #call, __FILE__, __LINE__)
/// Throw errno value if condition is not met
#define ErrnoAssert(condition)                                                 \
  if (!(condition)) {                                                          \
    ErrnoCodeThrow(errno, #condition);                                         \
  } else                                                                       \
    posix::err::Errno::_noop()

namespace posix {
namespace err {

/// @todo Test
class Errno : public msg::Exception {
public:
  /// tell us what the error is
  Errno(int value, const char *errnoName, const char *file = NULL,
        int line = 0) throw();
  /// tell us what the error is and a message
  Errno(const std::string &message, int value, const char *errnoName,
        const char *file = NULL, int line = 0) throw();
  /// Copy constructor
  Errno(const Errno &other);
  /// assignment operator
  Errno &operator=(const Errno &other);
  /// destructs _message
  virtual ~Errno() throw();
  /// Gets the name of the errno
  virtual const char *name() const;
  /// throws if errno is not 0
  int code() const throw();
  static void _throw(int errnoCode, const std::string &message,
                     const char *file, int line);
  static int _throwOnNegative(const int returnCode, const char *call,
                              const char *file, const int line);
  template <typename T>
  static T *_throwOnNull(T *returnAddress, const char *call, const char *file,
                         const int line);
  static void _noop();

private:
  int _errno; ///< The error code

  /// Helper function to initialize from the various constructors.
  template <class S>
  std::string _init(S message, const char *errnoName, int value);
};

/// Temporary define for declaring classes for each errno value
#define ErrnoException(errno_name)                                             \
  class errno_name##_Errno : public Errno {                                    \
  public:                                                                      \
    const char *name() const override { return #errno_name; }                  \
    explicit errno_name##_Errno(const char *file = NULL, int line = 0) throw() \
        : Errno(errno_name, #errno_name, file, line) {}                        \
    explicit errno_name##_Errno(const std::string &message,                    \
                                const char *file = NULL, int line = 0) throw() \
        : Errno(message, errno_name, #errno_name, file, line) {}               \
  }
ErrnoException(E2BIG);
ErrnoException(EACCES);
ErrnoException(EADDRINUSE);
ErrnoException(EADDRNOTAVAIL);
ErrnoException(EAFNOSUPPORT);
ErrnoException(EAGAIN);
ErrnoException(EALREADY);
ErrnoException(EBADF);
#if defined(EAUTH)
ErrnoException(EAUTH);
#endif
#if defined(EBADARCH)
ErrnoException(EBADARCH);
#endif
#if defined(EBADEXEC)
ErrnoException(EBADEXEC);
#endif
#if defined(EBADMACHO)
ErrnoException(EBADMACHO);
#endif
#if defined(EBADRPC)
ErrnoException(EBADRPC);
#endif
#if defined(EDEVERR)
ErrnoException(EDEVERR);
#endif
#if defined(EFTYPE)
ErrnoException(EFTYPE);
#endif
#if defined(ELAST)
ErrnoException(ELAST);
#endif
#if defined(ENEEDAUTH)
ErrnoException(ENEEDAUTH);
#endif
#if defined(ENOATTR)
ErrnoException(ENOATTR);
#endif
#if defined(ENOPOLICY)
ErrnoException(ENOPOLICY);
#endif
#if defined(EPROCLIM)
ErrnoException(EPROCLIM);
#endif
#if defined(EPROCUNAVAIL)
ErrnoException(EPROCUNAVAIL);
#endif
#if defined(EPROGMISMATCH)
ErrnoException(EPROGMISMATCH);
#endif
#if defined(EPROGUNAVAIL)
ErrnoException(EPROGUNAVAIL);
#endif
#if defined(EPWROFF)
ErrnoException(EPWROFF);
#endif
#if defined(ERPCMISMATCH)
ErrnoException(ERPCMISMATCH);
#endif
#if defined(ESHLIBVERS)
ErrnoException(ESHLIBVERS);
#endif
ErrnoException(EBADMSG);
ErrnoException(EBUSY);
ErrnoException(ECANCELED);
ErrnoException(ECHILD);
ErrnoException(ECONNABORTED);
ErrnoException(ECONNREFUSED);
ErrnoException(ECONNRESET);
ErrnoException(EDEADLK);
ErrnoException(EDESTADDRREQ);
ErrnoException(EDOM);
ErrnoException(EDQUOT);
ErrnoException(EEXIST);
ErrnoException(EFAULT);
ErrnoException(EFBIG);
ErrnoException(EHOSTDOWN);
ErrnoException(EHOSTUNREACH);
ErrnoException(EIDRM);
ErrnoException(EILSEQ);
ErrnoException(EINPROGRESS);
ErrnoException(EINTR);
ErrnoException(EINVAL);
ErrnoException(EIO);
ErrnoException(EISCONN);
ErrnoException(EISDIR);
ErrnoException(ELOOP);
ErrnoException(EMFILE);
ErrnoException(EMLINK);
ErrnoException(EMSGSIZE);
ErrnoException(EMULTIHOP);
ErrnoException(ENAMETOOLONG);
ErrnoException(ENETDOWN);
ErrnoException(ENETRESET);
ErrnoException(ENETUNREACH);
ErrnoException(ENFILE);
ErrnoException(ENOBUFS);
ErrnoException(ENODATA);
ErrnoException(ENODEV);
ErrnoException(ENOENT);
ErrnoException(ENOEXEC);
ErrnoException(ENOLCK);
ErrnoException(ENOLINK);
ErrnoException(ENOMEM);
ErrnoException(ENOMSG);
ErrnoException(ENOPROTOOPT);
ErrnoException(ENOSPC);
ErrnoException(ENOSR);
ErrnoException(ENOSTR);
ErrnoException(ENOSYS);
ErrnoException(ENOTBLK);
ErrnoException(ENOTCONN);
ErrnoException(ENOTDIR);
ErrnoException(ENOTEMPTY);
ErrnoException(ENOTSOCK);
ErrnoException(ENOTSUP);
ErrnoException(ENOTTY);
ErrnoException(ENXIO);
ErrnoException(EOPNOTSUPP);
ErrnoException(EOVERFLOW);
ErrnoException(EPERM);
ErrnoException(EPFNOSUPPORT);
ErrnoException(EPIPE);
ErrnoException(EPROTO);
ErrnoException(EPROTONOSUPPORT);
ErrnoException(EPROTOTYPE);
ErrnoException(ERANGE);
ErrnoException(EREMOTE);
ErrnoException(EROFS);
ErrnoException(ESHUTDOWN);
ErrnoException(ESOCKTNOSUPPORT);
ErrnoException(ESPIPE);
ErrnoException(ESRCH);
ErrnoException(ESTALE);
ErrnoException(ETIME);
ErrnoException(ETIMEDOUT);
ErrnoException(ETOOMANYREFS);
ErrnoException(ETXTBSY);
ErrnoException(EUSERS);
ErrnoException(EWOULDBLOCK); // not  tested
ErrnoException(EXDEV);
ErrnoException(ENOTRECOVERABLE); // not tested
ErrnoException(EOWNERDEAD);
#undef ErrnoException

inline Errno::Errno(int value, const char *errnoName, const char *file,
                    int line) throw() // not tested
    : msg::Exception(_init("", errnoName, value), file, line), _errno(value) {}
inline Errno::Errno(const std::string &message, int value,
                    const char *errnoName, const char *file, int line) throw()
    : msg::Exception(_init(message, errnoName, value), file, line),
      _errno(value) {}
inline Errno::Errno(const Errno &other) // not tested
    : msg::Exception(other), _errno(other._errno) {}
inline Errno &Errno::operator=(const Errno &other) { // not tested
  *reinterpret_cast<msg::Exception *>(this) = other;
  _errno = other._errno;
  return *this;
}
inline Errno::~Errno() throw() {}
inline const char *Errno::name() const { // not tested
  return "[Unknown]";                    // not tested
}
inline int Errno::code() const throw() {
  return _errno; // not tested
}
inline void Errno::_throw(int errnoCode, const std::string &message,
                          const char *file, int line) {
  if (0 != errnoCode) {
    switch (errnoCode) {
#define ErrnoCaseClass(name)                                                   \
  case name:                                                                   \
    throw name##_Errno(message, file, line)
      ErrnoCaseClass(E2BIG);
      ErrnoCaseClass(EACCES);
      ErrnoCaseClass(EADDRINUSE);
      ErrnoCaseClass(EADDRNOTAVAIL);
      ErrnoCaseClass(EAFNOSUPPORT);
      ErrnoCaseClass(EAGAIN);
      ErrnoCaseClass(EALREADY);
#if defined(EAUTH)
      ErrnoCaseClass(EAUTH);
#endif
#if defined(EBADARCH)
      ErrnoCaseClass(EBADARCH);
#endif
#if defined(EBADEXEC)
      ErrnoCaseClass(EBADEXEC);
#endif
#if defined(EBADMACHO)
      ErrnoCaseClass(EBADMACHO);
#endif
#if defined(EBADRPC)
      ErrnoCaseClass(EBADRPC);
#endif
#if defined(EDEVERR)
      ErrnoCaseClass(EDEVERR);
#endif
#if defined(EFTYPE)
      ErrnoCaseClass(EFTYPE);
#endif
#if defined(ELAST)
      ErrnoCaseClass(ELAST);
#endif
#if defined(ENEEDAUTH)
      ErrnoCaseClass(ENEEDAUTH);
#endif
#if defined(ENOATTR)
      ErrnoCaseClass(ENOATTR);
#endif
#if defined(EPROCLIM)
      ErrnoCaseClass(EPROCLIM);
#endif
#if defined(EPROCUNAVAIL)
      ErrnoCaseClass(EPROCUNAVAIL);
#endif
#if defined(EPROGMISMATCH)
      ErrnoCaseClass(EPROGMISMATCH);
#endif
#if defined(EPROGUNAVAIL)
      ErrnoCaseClass(EPROGUNAVAIL);
#endif
#if defined(EPWROFF)
      ErrnoCaseClass(EPWROFF);
#endif
#if defined(ERPCMISMATCH)
      ErrnoCaseClass(ERPCMISMATCH);
#endif
#if defined(ESHLIBVERS)
      ErrnoCaseClass(ESHLIBVERS);
#endif
#if defined(ENOPOLICY)
      ErrnoCaseClass(ENOPOLICY);
#endif
#if EOPNOTSUPP != ENOTSUP
      ErrnoCaseClass(EOPNOTSUPP);
#endif
#if EWOULDBLOCK != EAGAIN
      ErrnoCaseClass(EWOULDBLOCK);
#endif
      ErrnoCaseClass(EBADF);
      ErrnoCaseClass(EBADMSG);
      ErrnoCaseClass(EBUSY);
      ErrnoCaseClass(ECANCELED);
      ErrnoCaseClass(ECHILD);
      ErrnoCaseClass(ECONNABORTED);
      ErrnoCaseClass(ECONNREFUSED);
      ErrnoCaseClass(ECONNRESET);
      ErrnoCaseClass(EDEADLK);
      ErrnoCaseClass(EDESTADDRREQ);
      ErrnoCaseClass(EDOM);
      ErrnoCaseClass(EDQUOT);
      ErrnoCaseClass(EEXIST);
      ErrnoCaseClass(EFAULT);
      ErrnoCaseClass(EFBIG);
      ErrnoCaseClass(EHOSTDOWN);
      ErrnoCaseClass(EHOSTUNREACH);
      ErrnoCaseClass(EIDRM);
      ErrnoCaseClass(EILSEQ);
      ErrnoCaseClass(EINPROGRESS);
      ErrnoCaseClass(EINTR);
      ErrnoCaseClass(EINVAL);
      ErrnoCaseClass(EIO);
      ErrnoCaseClass(EISCONN);
      ErrnoCaseClass(EISDIR);
      ErrnoCaseClass(ELOOP);
      ErrnoCaseClass(EMFILE);
      ErrnoCaseClass(EMLINK);
      ErrnoCaseClass(EMSGSIZE);
      ErrnoCaseClass(EMULTIHOP);
      ErrnoCaseClass(ENAMETOOLONG);
      ErrnoCaseClass(ENETDOWN);
      ErrnoCaseClass(ENETRESET);
      ErrnoCaseClass(ENETUNREACH);
      ErrnoCaseClass(ENFILE);
      ErrnoCaseClass(ENOBUFS);
      ErrnoCaseClass(ENODATA);
      ErrnoCaseClass(ENODEV);
      ErrnoCaseClass(ENOENT);
      ErrnoCaseClass(ENOEXEC);
      ErrnoCaseClass(ENOLCK);
      ErrnoCaseClass(ENOLINK);
      ErrnoCaseClass(ENOMEM);
      ErrnoCaseClass(ENOMSG);
      ErrnoCaseClass(ENOPROTOOPT);
      ErrnoCaseClass(ENOSPC);
      ErrnoCaseClass(ENOSR);
      ErrnoCaseClass(ENOSTR);
      ErrnoCaseClass(ENOSYS);
      ErrnoCaseClass(ENOTBLK);
      ErrnoCaseClass(ENOTCONN);
      ErrnoCaseClass(ENOTDIR);
      ErrnoCaseClass(ENOTEMPTY);
      ErrnoCaseClass(ENOTSOCK);
      ErrnoCaseClass(ENOTSUP);
      ErrnoCaseClass(ENOTTY);
      ErrnoCaseClass(ENXIO);
      ErrnoCaseClass(EOVERFLOW);
      ErrnoCaseClass(EOWNERDEAD);
      ErrnoCaseClass(EPERM);
      ErrnoCaseClass(EPFNOSUPPORT);
      ErrnoCaseClass(EPIPE);
      ErrnoCaseClass(EPROTO);
      ErrnoCaseClass(EPROTONOSUPPORT);
      ErrnoCaseClass(EPROTOTYPE);
      ErrnoCaseClass(ERANGE);
      ErrnoCaseClass(EREMOTE);
      ErrnoCaseClass(EROFS);
      ErrnoCaseClass(ESHUTDOWN);
      ErrnoCaseClass(ESOCKTNOSUPPORT);
      ErrnoCaseClass(ESPIPE);
      ErrnoCaseClass(ESRCH);
      ErrnoCaseClass(ESTALE);
      ErrnoCaseClass(ETIME);
      ErrnoCaseClass(ETIMEDOUT);
      ErrnoCaseClass(ETOOMANYREFS);
      ErrnoCaseClass(ETXTBSY);
      ErrnoCaseClass(EUSERS);
      ErrnoCaseClass(EXDEV);
      ErrnoCaseClass(ENOTRECOVERABLE); // not tested
#undef ErrnoCaseClass
    default:
      throw Errno(message, errnoCode, "[Unknown]", file, line); // not tested
    }
  }
}
inline int Errno::_throwOnNegative(const int returnCode, const char *call,
                                   const char *file, const int line) {
  if (returnCode < 0) {
    _throw(errno, call, file, line); // tested on Darwin and libernet
  }
  return returnCode;
}
template <typename T>
inline T *Errno::_throwOnNull(T *returnAddress, const char *call,
                              const char *file, const int line) {
  if (nullptr == returnAddress) {
    _throw(errno, call, file, line);
  }
  return returnAddress;
}
inline void Errno::_noop() {}
template <class S>
inline std::string Errno::_init(S message, const char *errnoName, int value) {
  return std::string("[") + errnoName + " (" + std::to_string(value) +
         "): " + strerror(value) + "]: " + std::string(message);
}

} // namespace err
} // namespace posix

#endif // __POSIXErrno_h__
