#define USE_DEPRECATED_ERRNO_EXCEPTIONS
#include "os/Exception.h"
#include "os/POSIXErrno.h"

#define fail fprintf(stderr, "FAIL: %s:%d\n", __FILE__, __LINE__);

#define Test(errno_name) \
	try { \
		ErrnoCodeThrow(errno_name, "Testing " #errno_name); \
		fail \
	} catch(const posix::err::errno_name##_Errno &) { \
	} catch(const std::exception &exception) { \
		fprintf(stderr, "%s\n", exception.what()); \
		fail \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	int	iterations= 1500;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		//compileTimeAssert(sizeof(int) == sizeof(char));
		compileTimeAssert(sizeof(int) == sizeof(unsigned int));
		void		*null= NULL;
		void		*notNull= &null;
		const int	noerror= 0;
		const int	error= ERANGE;

		Test(E2BIG);			Test(EACCES);			Test(EADDRINUSE);
		Test(EADDRNOTAVAIL);	Test(EAFNOSUPPORT);		Test(EAGAIN);
		Test(EALREADY);			Test(EAUTH);			Test(EBADARCH);
		Test(EBADEXEC);			Test(EBADF);			Test(EBADMACHO);
		Test(EBADMSG);			Test(EBADRPC);			Test(EBUSY);
		Test(ECANCELED);		Test(ECHILD);			Test(ECONNABORTED);
		Test(ECONNREFUSED);		Test(ECONNRESET);		Test(EDEADLK);
		Test(EDESTADDRREQ);		Test(EDEVERR);			Test(EDOM);
		Test(EDQUOT);			Test(EEXIST);			Test(EFAULT);
		Test(EFBIG);			Test(EFTYPE);			Test(EHOSTDOWN);
		Test(EHOSTUNREACH);		Test(EIDRM);			Test(EILSEQ);
		Test(EINPROGRESS);		Test(EINTR);			Test(EINVAL);
		Test(EIO);				Test(EISCONN);			Test(EISDIR);
		Test(ELAST);			Test(ELOOP);			Test(EMFILE);
		Test(EMLINK);			Test(EMSGSIZE);			Test(EMULTIHOP);
		Test(ENAMETOOLONG);		Test(ENEEDAUTH);		Test(ENETDOWN);
		Test(ENETRESET);		Test(ENETUNREACH);		Test(ENFILE);
		Test(ENOATTR);			Test(ENOBUFS);			Test(ENODATA);
		Test(ENODEV);			Test(ENOENT);			Test(ENOEXEC);
		Test(ENOLCK);			Test(ENOLINK);			Test(ENOMEM);
		Test(ENOMSG);			/*Test(ENOPOLICY);*/	Test(ENOPROTOOPT);
		Test(ENOSPC);			Test(ENOSR);			Test(ENOSTR);
		Test(ENOSYS);			Test(ENOTBLK);			Test(ENOTCONN);
		Test(ENOTDIR);			Test(ENOTEMPTY);
		Test(ENOTSOCK);			Test(ENOTSUP);			Test(ENOTTY);
		Test(ENXIO);			Test(EOPNOTSUPP);		Test(EOVERFLOW);
		Test(EPERM);			Test(EPFNOSUPPORT);
		Test(EPIPE);			Test(EPROCLIM);			Test(EPROCUNAVAIL);
		Test(EPROGMISMATCH);	Test(EPROGUNAVAIL);		Test(EPROTO);
		Test(EPROTONOSUPPORT);	Test(EPROTOTYPE);		Test(EPWROFF);
		Test(ERANGE);			Test(EREMOTE);			Test(EROFS);
		Test(ERPCMISMATCH);		Test(ESHLIBVERS);		Test(ESHUTDOWN);
		Test(ESOCKTNOSUPPORT);	Test(ESPIPE);			Test(ESRCH);
		Test(ESTALE);			Test(ETIME);			Test(ETIMEDOUT);
		Test(ETOOMANYREFS);		Test(ETXTBSY);			Test(EUSERS);
		/*Test(EWOULDBLOCK);*/	Test(EXDEV);

		try	{
			ThrowMessageException("Testing");
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			ThrowMessageExceptionIfNULL(null);
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			ThrowMessageExceptionIfNULL(notNull);
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
			fail
		}
		try	{
			AssertMessageException(null == notNull);
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			AssertMessageException(null != notNull);
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
			fail
		}
		try	{
			AssertCodeMessageException(error);
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			AssertCodeMessageException(noerror);
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
			fail
		}
		try	{
			errnoThrowMessageException(error, "error");
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			errnoThrowMessageException(noerror, "no error");
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			errnoCodeThrowMessageException(error, "error");
			fail
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
		}
		try	{
			errnoCodeThrowMessageException(noerror, "no error");
		} catch(const std::exception &exception) {
			printf("exception='%s'\n",exception.what());
			fail
		}
	}
	return 0;
}
