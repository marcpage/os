#include "os/POSIXErrno.h"
#include <stdio.h>

int main(const int, const char *const[]) {
  int iterations = 400;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  int errors[] = {
    0,
    E2BIG,
    EACCES,
    EADDRINUSE,
    EADDRNOTAVAIL,
    EAFNOSUPPORT,
    EAGAIN,
    EALREADY,
#if defined(EAUTH)
    EAUTH,
#endif
#if defined(EBADARCH)
    EBADARCH,
#endif
#if defined(EBADEXEC)
    EBADEXEC,
#endif
#if defined(EBADMACHO)
    EBADMACHO,
#endif
#if defined(EBADRPC)
    EBADRPC,
#endif
#if defined(EDEVERR)
    EDEVERR,
#endif
#if defined(EFTYPE)
    EFTYPE,
#endif
#if defined(ELAST)
    ELAST,
#endif
#if defined(ENEEDAUTH)
    ENEEDAUTH,
#endif
#if defined(ENOATTR)
    ENOATTR,
#endif
#if defined(EPROCLIM)
    EPROCLIM,
#endif
#if defined(EPROCUNAVAIL)
    EPROCUNAVAIL,
#endif
#if defined(EPROGMISMATCH)
    EPROGMISMATCH,
#endif
#if defined(EPROGUNAVAIL)
    EPROGUNAVAIL,
#endif
#if defined(EPWROFF)
    EPWROFF,
#endif
#if defined(ERPCMISMATCH)
    ERPCMISMATCH,
#endif
#if defined(ESHLIBVERS)
    ESHLIBVERS,
#endif
#if defined(ENOPOLICY)
    ENOPOLICY,
#endif
#if EOPNOTSUPP != ENOTSUP
    EOPNOTSUPP,
#endif
#if EWOULDBLOCK != EAGAIN
    EWOULDBLOCK,
#endif
    EBADF,
    EBADMSG,
    EBUSY,
    ECANCELED,
    ECHILD,
    ECONNABORTED,
    ECONNREFUSED,
    ECONNRESET,
    EDEADLK,
    EDESTADDRREQ,
    EDOM,
    EDQUOT,
    EEXIST,
    EFAULT,
    EFBIG,
    EHOSTDOWN,
    EHOSTUNREACH,
    EIDRM,
    EILSEQ,
    EINPROGRESS,
    EINTR,
    EINVAL,
    EIO,
    EISCONN,
    EISDIR,
    ELOOP,
    EMFILE,
    EMLINK,
    EMSGSIZE,
    EMULTIHOP,
    ENAMETOOLONG,
    ENETDOWN,
    ENETRESET,
    ENETUNREACH,
    ENFILE,
    ENOBUFS,
    ENODATA,
    ENODEV,
    ENOENT,
    ENOEXEC,
    ENOLCK,
    ENOLINK,
    ENOMEM,
    ENOMSG,
    ENOPROTOOPT,
    ENOSPC,
    ENOSR,
    ENOSTR,
    ENOSYS,
    ENOTBLK,
    ENOTCONN,
    ENOTDIR,
    ENOTEMPTY,
    ENOTSOCK,
    ENOTSUP,
    ENOTTY,
    ENXIO,
    EOVERFLOW,
    EOWNERDEAD,
    EPERM,
    EPFNOSUPPORT,
    EPIPE,
    EPROTO,
    EPROTONOSUPPORT,
    EPROTOTYPE,
    ERANGE,
    EREMOTE,
    EROFS,
    ESHUTDOWN,
    ESOCKTNOSUPPORT,
    ESPIPE,
    ESRCH,
    ESTALE,
    ETIME,
    ETIMEDOUT,
    ETOOMANYREFS,
    ETXTBSY,
    EUSERS,
    EXDEV
  };

  for (int i = 0; i < iterations; ++i) {
    for (unsigned int code = 0; code < sizeof(errors) / sizeof(errors[0]);
         ++code) {
      try {
        ErrnoCodeThrow(errors[code], "test");
        if (0 != errors[code]) {
          fprintf(stderr,
                  "FAIL(%s:%d): Expected to throw an exception, but didn't "
                  "(code=%d, value=%d)\n",
                  __FILE__, __LINE__, code, errors[code]);
        }
      } catch (const posix::err::Errno &exception) {
        if (0 == errors[code]) {
          fprintf(stderr, "FAIL(%s:%d): Zero Exception caught: %s\n", __FILE__,
                  __LINE__, exception.what());
        } else {
          printf("Caught Exception: %s\n", exception.name());
        }
      } catch (const std::exception &exception) {
        fprintf(stderr, "FAIL(%s:%d): Generic Exception caught: %s\n", __FILE__,
                __LINE__, exception.what());
      }
    }
  }
  return 0;
}

/*
 */
