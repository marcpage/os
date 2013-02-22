#ifndef __POSIXErrno_h__
#define __POSIXErrno_h__

#include <exception>
#include <string>
#include <errno.h>
#include <sstream>
#include <string.h>

/// Throw by errno error code name
#define ErrnoThrowAssert(condition, name) throw posix::err::name##_Errno(#condition, __FILE__, __LINE__)
/// Throw by errno error code name
#define ErrnoThrow(name) throw posix::err::name##_Errno(#name, __FILE__, __LINE__)
/// Throw by errno error code numeric value
#define ErrnoCodeThrow(errnoCode, message) posix::err::Errno::_throw(errnoCode, message, __FILE__, __LINE__)
/// Throw if errno is not 0
#define ErrnoMessageThrow(message) posix::err::Errno::_throw(errno, message, __FILE__, __LINE__)
/// Wrap around a call that sets errno on negative return value
#define ErrnoOnNegative(call) if( (call) < 0) {ErrnoCodeThrow(errno, #call);} else posix::err::Errno::_noop()
/// Wrap around a call that sets errno on NULL return value
#define ErrnoOnNULL(call) if( NULL == (call) ) {ErrnoCodeThrow(errno, #call);} else posix::err::Errno::_noop()
/// Throw errno value if condition is not met
#define ErrnoAssert(condition) if(!(condition)) {ErrnoCodeThrow(errno, #condition);} else posix::err::Errno::_noop()

/**
	@todo Test!
*/
namespace posix { namespace err {

	class Errno : public std::exception {
		public:
			/// tell us what the error is
			Errno(int value, const char *file= NULL, int line= 0) throw();
			/// tell us what the error is and a message
			Errno(const std::string &message, int value, const char *file= NULL, int line= 0) throw();
			/// Copy constructor
			Errno(const Errno &other);
			/// assignment operator
			Errno &operator=(const Errno &other);
			/// destructs _message
			virtual ~Errno() throw();
			/// gets the message
			virtual const char* what() const throw();
			/// throws if errno is not 0
			int code() const throw();
			static void _throw(int errnoCode, const std::string &message, const char *file, int line);
			static void _noop();
		private:
			std::string	*_message;	///< The message about why the exception was thrown.
			int			_errno;		///< The error code

			/// Helper function to initialize from the various constructors.
			template<class S> std::string *_init(S message, const char *file, int line, int value);
	};

/// Temporary define for declaring classes for each errno value
#define ErrnoException(name) \
	class name##_Errno : public Errno { \
		public: \
		name##_Errno(const char *file= NULL, int line= 0) throw():Errno(name, file, line) {} \
		name##_Errno(const std::string &message, const char *file= NULL, int line= 0) throw():Errno(message, name, file, line) {} \
	}
	ErrnoException(E2BIG);			ErrnoException(EACCES);			ErrnoException(EADDRINUSE);
	ErrnoException(EADDRNOTAVAIL);	ErrnoException(EAFNOSUPPORT);	ErrnoException(EAGAIN);
	ErrnoException(EALREADY);		ErrnoException(EAUTH);			ErrnoException(EBADARCH);
	ErrnoException(EBADEXEC);		ErrnoException(EBADF);			ErrnoException(EBADMACHO);
	ErrnoException(EBADMSG);		ErrnoException(EBADRPC);		ErrnoException(EBUSY);
	ErrnoException(ECANCELED);		ErrnoException(ECHILD);			ErrnoException(ECONNABORTED);
	ErrnoException(ECONNREFUSED);	ErrnoException(ECONNRESET);		ErrnoException(EDEADLK);
	ErrnoException(EDESTADDRREQ);	ErrnoException(EDEVERR);		ErrnoException(EDOM);
	ErrnoException(EDQUOT);			ErrnoException(EEXIST);			ErrnoException(EFAULT);
	ErrnoException(EFBIG);			ErrnoException(EFTYPE);			ErrnoException(EHOSTDOWN);
	ErrnoException(EHOSTUNREACH);	ErrnoException(EIDRM);			ErrnoException(EILSEQ);
	ErrnoException(EINPROGRESS);	ErrnoException(EINTR);			ErrnoException(EINVAL);
	ErrnoException(EIO);			ErrnoException(EISCONN);		ErrnoException(EISDIR);
	ErrnoException(ELAST);			ErrnoException(ELOOP);			ErrnoException(EMFILE);
	ErrnoException(EMLINK);			ErrnoException(EMSGSIZE);		ErrnoException(EMULTIHOP);
	ErrnoException(ENAMETOOLONG);	ErrnoException(ENEEDAUTH);		ErrnoException(ENETDOWN);
	ErrnoException(ENETRESET);		ErrnoException(ENETUNREACH);	ErrnoException(ENFILE);
	ErrnoException(ENOATTR);		ErrnoException(ENOBUFS);		ErrnoException(ENODATA);
	ErrnoException(ENODEV);			ErrnoException(ENOENT);			ErrnoException(ENOEXEC);
	ErrnoException(ENOLCK);			ErrnoException(ENOLINK);		ErrnoException(ENOMEM);
	ErrnoException(ENOMSG);			ErrnoException(ENOPOLICY);		ErrnoException(ENOPROTOOPT);
	ErrnoException(ENOSPC);			ErrnoException(ENOSR);			ErrnoException(ENOSTR);
	ErrnoException(ENOSYS);			ErrnoException(ENOTBLK);		ErrnoException(ENOTCONN);
	ErrnoException(ENOTDIR);		ErrnoException(ENOTEMPTY);
	ErrnoException(ENOTSOCK);		ErrnoException(ENOTSUP);		ErrnoException(ENOTTY);
	ErrnoException(ENXIO);			ErrnoException(EOPNOTSUPP);		ErrnoException(EOVERFLOW);
	ErrnoException(EPERM);			ErrnoException(EPFNOSUPPORT);
	ErrnoException(EPIPE);			ErrnoException(EPROCLIM);		ErrnoException(EPROCUNAVAIL);
	ErrnoException(EPROGMISMATCH);	ErrnoException(EPROGUNAVAIL);	ErrnoException(EPROTO);
	ErrnoException(EPROTONOSUPPORT);ErrnoException(EPROTOTYPE);		ErrnoException(EPWROFF);
	ErrnoException(ERANGE);			ErrnoException(EREMOTE);		ErrnoException(EROFS);
	ErrnoException(ERPCMISMATCH);	ErrnoException(ESHLIBVERS);		ErrnoException(ESHUTDOWN);
	ErrnoException(ESOCKTNOSUPPORT);ErrnoException(ESPIPE);			ErrnoException(ESRCH);
	ErrnoException(ESTALE);			ErrnoException(ETIME);			ErrnoException(ETIMEDOUT);
	ErrnoException(ETOOMANYREFS);	ErrnoException(ETXTBSY);		ErrnoException(EUSERS);
	ErrnoException(EWOULDBLOCK);	ErrnoException(EXDEV);
#if 0
	ErrnoException(ENOTRECOVERABLE);
	ErrnoException(EOWNERDEAD);
#endif
#undef ErrnoException

	inline Errno::Errno(int value, const char *file, int line) throw()
		:_message(_init("", file, line, value)), _errno(value) {}
	inline Errno::Errno(const std::string &message, int value, const char *file, int line) throw()
		:_message(_init(message, file, line, value)), _errno(value) {}
	inline Errno::Errno(const Errno &other)
		:std::exception(), _message(other._message), _errno(other._errno) {
		if(NULL != _message) {
			try {
				_message= new std::string(*_message);
			} catch(const std::exception&) {
				_message= NULL;
			}
		}
	}
	inline Errno &Errno::operator=(const Errno &other) {
		if(this != &other) {
			if(NULL != other._message) {
				try {
					if(NULL != _message) {
						_message->assign(*other._message);
					} else {
						_message= new std::string(*other._message);
					}
				} catch(const std::exception&) {
				}
			} else {
				delete _message;
				_message= NULL;
			}
		}
		return *this;
	}
	inline Errno::~Errno() throw() {
		delete _message;
		_message= NULL;
	}
	inline const char* Errno::what() const throw() {
		if(NULL != _message) {
			return _message->c_str();
		} else {
			return "Unable to allocate message string at exception throw!";
		}
	}
	inline int Errno::code() const throw() {
		return _errno;
	}
	inline void Errno::_throw(int errnoCode, const std::string &message, const char *file, int line) {
		if(0 != errnoCode) {
			switch(errnoCode) {
#define ErrnoCaseClass(name) case name: throw name##_Errno(message, file, line)
		ErrnoCaseClass(E2BIG);			ErrnoCaseClass(EACCES);			ErrnoCaseClass(EADDRINUSE);
		ErrnoCaseClass(EADDRNOTAVAIL);	ErrnoCaseClass(EAFNOSUPPORT);	ErrnoCaseClass(EAGAIN);
		ErrnoCaseClass(EALREADY);		ErrnoCaseClass(EAUTH);			ErrnoCaseClass(EBADARCH);
		ErrnoCaseClass(EBADEXEC);		ErrnoCaseClass(EBADF);			ErrnoCaseClass(EBADMACHO);
		ErrnoCaseClass(EBADMSG);		ErrnoCaseClass(EBADRPC);		ErrnoCaseClass(EBUSY);
		ErrnoCaseClass(ECANCELED);		ErrnoCaseClass(ECHILD);			ErrnoCaseClass(ECONNABORTED);
		ErrnoCaseClass(ECONNREFUSED);	ErrnoCaseClass(ECONNRESET);		ErrnoCaseClass(EDEADLK);
		ErrnoCaseClass(EDESTADDRREQ);	ErrnoCaseClass(EDEVERR);		ErrnoCaseClass(EDOM);
		ErrnoCaseClass(EDQUOT);			ErrnoCaseClass(EEXIST);			ErrnoCaseClass(EFAULT);
		ErrnoCaseClass(EFBIG);			ErrnoCaseClass(EFTYPE);			ErrnoCaseClass(EHOSTDOWN);
		ErrnoCaseClass(EHOSTUNREACH);	ErrnoCaseClass(EIDRM);			ErrnoCaseClass(EILSEQ);
		ErrnoCaseClass(EINPROGRESS);	ErrnoCaseClass(EINTR);			ErrnoCaseClass(EINVAL);
		ErrnoCaseClass(EIO);			ErrnoCaseClass(EISCONN);		ErrnoCaseClass(EISDIR);
		ErrnoCaseClass(ELAST);			ErrnoCaseClass(ELOOP);			ErrnoCaseClass(EMFILE);
		ErrnoCaseClass(EMLINK);			ErrnoCaseClass(EMSGSIZE);		ErrnoCaseClass(EMULTIHOP);
		ErrnoCaseClass(ENAMETOOLONG);	ErrnoCaseClass(ENEEDAUTH);		ErrnoCaseClass(ENETDOWN);
		ErrnoCaseClass(ENETRESET);		ErrnoCaseClass(ENETUNREACH);	ErrnoCaseClass(ENFILE);
		ErrnoCaseClass(ENOATTR);		ErrnoCaseClass(ENOBUFS);		ErrnoCaseClass(ENODATA);
		ErrnoCaseClass(ENODEV);			ErrnoCaseClass(ENOENT);			ErrnoCaseClass(ENOEXEC);
		ErrnoCaseClass(ENOLCK);			ErrnoCaseClass(ENOLINK);		ErrnoCaseClass(ENOMEM);
		ErrnoCaseClass(ENOMSG);			/*ErrnoCaseClass(ENOPOLICY);*/	ErrnoCaseClass(ENOPROTOOPT);
		ErrnoCaseClass(ENOSPC);			ErrnoCaseClass(ENOSR);			ErrnoCaseClass(ENOSTR);
		ErrnoCaseClass(ENOSYS);			ErrnoCaseClass(ENOTBLK);		ErrnoCaseClass(ENOTCONN);
		ErrnoCaseClass(ENOTDIR);		ErrnoCaseClass(ENOTEMPTY);
		ErrnoCaseClass(ENOTSOCK);		ErrnoCaseClass(ENOTSUP);		ErrnoCaseClass(ENOTTY);
		ErrnoCaseClass(ENXIO);			ErrnoCaseClass(EOPNOTSUPP);		ErrnoCaseClass(EOVERFLOW);
		/*ErrnoCaseClass(EOWNERDEAD);*/	ErrnoCaseClass(EPERM);			ErrnoCaseClass(EPFNOSUPPORT);
		ErrnoCaseClass(EPIPE);			ErrnoCaseClass(EPROCLIM);		ErrnoCaseClass(EPROCUNAVAIL);
		ErrnoCaseClass(EPROGMISMATCH);	ErrnoCaseClass(EPROGUNAVAIL);	ErrnoCaseClass(EPROTO);
		ErrnoCaseClass(EPROTONOSUPPORT);ErrnoCaseClass(EPROTOTYPE);		ErrnoCaseClass(EPWROFF);
		ErrnoCaseClass(ERANGE);			ErrnoCaseClass(EREMOTE);		ErrnoCaseClass(EROFS);
		ErrnoCaseClass(ERPCMISMATCH);	ErrnoCaseClass(ESHLIBVERS);		ErrnoCaseClass(ESHUTDOWN);
		ErrnoCaseClass(ESOCKTNOSUPPORT);ErrnoCaseClass(ESPIPE);			ErrnoCaseClass(ESRCH);
		ErrnoCaseClass(ESTALE);			ErrnoCaseClass(ETIME);			ErrnoCaseClass(ETIMEDOUT);
		ErrnoCaseClass(ETOOMANYREFS);	ErrnoCaseClass(ETXTBSY);		ErrnoCaseClass(EUSERS);
		/*ErrnoCaseClass(EWOULDBLOCK);*/ErrnoCaseClass(EXDEV);
#if 0
		ErrnoCaseClass(ENOTRECOVERABLE);
#endif
#undef ErrnoCase
				default:
					throw Errno(message, errnoCode, file, line);
			}
		}
	}
	inline void Errno::_noop() {}
	template<class S> inline std::string *Errno::_init(S message, const char *file, int line, int value) {
		std::string	*messagePtr= NULL;
		try {
			messagePtr = new std::string(message);
			if(NULL != file) {
				messagePtr->append(" File: ").append(file);
				if(0 != line) {
					std::ostringstream	stream;

					stream << line;
					messagePtr->append(" Line: ").append(stream.str());
				}
				if(0 != value) {
					std::ostringstream	stream;

					stream << value;
					stream << " : ";
					stream << strerror(value);
					messagePtr->append(" Errno: ").append(stream.str());
				}
			}
		} catch (const std::exception&) {
		}
		return messagePtr;
	}

}}

#endif // __POSIXErrno_h__
