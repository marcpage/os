#include <stdio.h>
#include "os/POSIXErrno.h"

int main(const int , const char * const []) {
	int	iterations= 300;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	int errors[] = {
		E2BIG, EACCES, EADDRINUSE, EADDRNOTAVAIL, EAFNOSUPPORT, EAGAIN, EALREADY, EAUTH,
		EBADARCH, EBADEXEC, EBADF, EBADMACHO, EBADMSG, EBADRPC, EBUSY, ECANCELED, ECHILD,
		ECONNABORTED, ECONNREFUSED, ECONNRESET, EDEADLK, EDESTADDRREQ, EDEVERR, EDOM,
		EDQUOT, EEXIST, EFAULT, EFBIG, EFTYPE, EHOSTDOWN, EHOSTUNREACH, EIDRM, EILSEQ,
		EINPROGRESS, EINTR, EINVAL, EIO, EISCONN, EISDIR, ELAST, ELOOP, EMFILE, EMLINK,
		EMSGSIZE, EMULTIHOP, ENAMETOOLONG, ENEEDAUTH, ENETDOWN, ENETRESET, ENETUNREACH,
		ENFILE, ENOATTR, ENOBUFS, ENODATA, ENODEV, ENOENT, ENOEXEC, ENOLCK, ENOLINK,
		ENOMEM, ENOMSG, ENOPOLICY, ENOPROTOOPT, ENOSPC, ENOSR, ENOSTR, ENOSYS, ENOTBLK,
		ENOTCONN, ENOTDIR, ENOTEMPTY, ENOTSOCK, ENOTSUP, ENOTTY, ENXIO, EOPNOTSUPP,
		EOVERFLOW, EOWNERDEAD, EPERM, EPFNOSUPPORT, EPIPE, EPROCLIM, EPROCUNAVAIL,
		EPROGMISMATCH, EPROGUNAVAIL, EPROTO, EPROTONOSUPPORT, EPROTOTYPE, EPWROFF, ERANGE,
		EREMOTE, EROFS, ERPCMISMATCH, ESHLIBVERS, ESHUTDOWN, ESOCKTNOSUPPORT, ESPIPE,
		ESRCH, ESTALE, ETIME, ETIMEDOUT, ETOOMANYREFS, ETXTBSY, EUSERS, EWOULDBLOCK, EXDEV
	};

	for (int i = 0; i < iterations; ++i) {
		for (unsigned int code = 0; code < sizeof(errors)/sizeof(errors[0]); ++code) {
			try {
				ErrnoCodeThrow(errors[code], "test");
			} catch(const posix::err::Errno &exception) {
				printf("Exception caught: %s: %s\n", exception.name(), exception.what());
			} catch(const std::exception &exception) {
				fprintf(stderr, "FAIL(%s:%d): Generic Exception caught: %s\n",__FILE__, __LINE__, exception.what());
			}
		}
	}
	return 0;
}

/*
		 */
