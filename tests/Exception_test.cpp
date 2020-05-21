#include "os/Exception.h"
#include "os/POSIXErrno.h"

#define fail fprintf(stderr, "FAIL: %s:%d\n", __FILE__, __LINE__);

#define Test(errno_name)                                                       \
  try {                                                                        \
    ErrnoCodeThrow(errno_name, "Testing " #errno_name);                        \
    fail                                                                       \
  } catch (const posix::err::errno_name##_Errno &) {                           \
  } catch (const std::exception &exception) {                                  \
    fprintf(stderr, "%s\n", exception.what());                                 \
    fail                                                                       \
  }

int main(const int /*argc*/, const char *const /*argv*/[]) {
  int iterations = 350;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    // compileTimeAssert(sizeof(int) == sizeof(char));
    compileTimeAssert(sizeof(int) == sizeof(unsigned int));
    void *null = NULL;
    void *notNull = &null;
    const int noerror = 0;
    const int error = ERANGE;

    Test(E2BIG);
    Test(EACCES);
    Test(EADDRINUSE);
    Test(EADDRNOTAVAIL);
    Test(EAFNOSUPPORT);
    Test(EAGAIN);
    Test(EALREADY);
#if defined(EAUTH)
    Test(EAUTH);
#endif
#if defined(EBADARCH)
    Test(EBADARCH);
#endif
#if defined(EBADEXEC)
    Test(EBADEXEC);
#endif
#if defined(EBADMACHO)
    Test(EBADMACHO);
#endif
#if defined(EBADRPC)
    Test(EBADRPC);
#endif
#if defined(EDEVERR)
    Test(EDEVERR);
#endif
#if defined(EFTYPE)
    Test(EFTYPE);
#endif
#if defined(ELAST)
    Test(ELAST);
#endif
#if defined(ENEEDAUTH)
    Test(ENEEDAUTH);
#endif
#if defined(ENOATTR)
    Test(ENOATTR);
#endif
#if defined(EPROCLIM)
    Test(EPROCLIM);
#endif
#if defined(EPROCUNAVAIL)
    Test(EPROCUNAVAIL);
#endif
#if defined(EPROGMISMATCH)
    Test(EPROGMISMATCH);
#endif
#if defined(EPROGUNAVAIL)
    Test(EPROGUNAVAIL);
#endif
#if defined(EPWROFF)
    Test(EPWROFF);
#endif
#if defined(ERPCMISMATCH)
    Test(ERPCMISMATCH);
#endif
#if defined(ESHLIBVERS)
    Test(ESHLIBVERS);
#endif
#if defined(ENOPOLICY)
    Test(ENOPOLICY);
#endif
#if EOPNOTSUPP != ENOTSUP
    Test(EOPNOTSUPP);
#endif
#if EWOULDBLOCK != EAGAIN
    Test(EWOULDBLOCK);
#endif
    Test(EBADF);
    Test(EBADMSG);
    Test(EBUSY);
    Test(ECANCELED);
    Test(ECHILD);
    Test(ECONNABORTED);
    Test(ECONNREFUSED);
    Test(ECONNRESET);
    Test(EDEADLK);
    Test(EDESTADDRREQ);
    Test(EDOM);
    Test(EDQUOT);
    Test(EEXIST);
    Test(EFAULT);
    Test(EFBIG);
    Test(EHOSTDOWN);
    Test(EHOSTUNREACH);
    Test(EIDRM);
    Test(EILSEQ);
    Test(EINPROGRESS);
    Test(EINTR);
    Test(EINVAL);
    Test(EIO);
    Test(EISCONN);
    Test(EISDIR);
    Test(ELOOP);
    Test(EMFILE);
    Test(EMLINK);
    Test(EMSGSIZE);
    Test(EMULTIHOP);
    Test(ENAMETOOLONG);
    Test(ENETDOWN);
    Test(ENETRESET);
    Test(ENETUNREACH);
    Test(ENFILE);
    Test(ENOBUFS);
    Test(ENODATA);
    Test(ENODEV);
    Test(ENOENT);
    Test(ENOEXEC);
    Test(ENOLCK);
    Test(ENOLINK);
    Test(ENOMEM);
    Test(ENOMSG);
    Test(ENOPROTOOPT);
    Test(ENOSPC);
    Test(ENOSR);
    Test(ENOSTR);
    Test(ENOSYS);
    Test(ENOTBLK);
    Test(ENOTCONN);
    Test(ENOTDIR);
    Test(ENOTEMPTY);
    Test(ENOTSOCK);
    Test(ENOTSUP);
    Test(ENOTTY);
    Test(ENXIO);
    Test(EOVERFLOW);
    Test(EPERM);
    Test(EPFNOSUPPORT);
    Test(EPIPE);
    Test(EPROTO);
    Test(EPROTONOSUPPORT);
    Test(EPROTOTYPE);
    Test(ERANGE);
    Test(EREMOTE);
    Test(EROFS);
    Test(ESHUTDOWN);
    Test(ESOCKTNOSUPPORT);
    Test(ESPIPE);
    Test(ESRCH);
    Test(ESTALE);
    Test(ETIME);
    Test(ETIMEDOUT);
    Test(ETOOMANYREFS);
    Test(ETXTBSY);
    Test(EUSERS);
    Test(EXDEV);

    try {
      ThrowMessageException("Testing");
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      ThrowMessageExceptionIfNULL(null);
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      ThrowMessageExceptionIfNULL(notNull);
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
      fail
    }
    try {
      AssertMessageException(null == notNull);
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      AssertMessageException(null != notNull);
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
      fail
    }
    try {
      AssertCodeMessageException(error);
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      AssertCodeMessageException(noerror);
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
      fail
    }
    try {
      ErrnoCodeThrow(error, "error");
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      ErrnoCodeThrow(noerror, "no error");
    } catch (const std::exception &exception) {
      fail printf("exception='%s'\n", exception.what());
    }
    try {
      ErrnoCodeThrow(error, "error");
      fail
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
    }
    try {
      ErrnoCodeThrow(noerror, "no error");
    } catch (const std::exception &exception) {
      printf("exception='%s'\n", exception.what());
      fail
    }
  }
  return 0;
}
