#ifndef __MessageException_h__
#define __MessageException_h__

#include <string>
#include <exception>
#include <sstream>
#include <errno.h>

inline void noop() {}

#define ThrowMessageException(message) throw msg::Exception((message), __FILE__, __LINE__)
#define ThrowMessageExceptionIfNULL(variable) if(NULL == variable) {ThrowMessageException(std::string(#variable).append(" == NULL"));} else noop()
#define AssertMessageException(condition) if(!(condition)) {ThrowMessageException(#condition);} else noop()
#define AssertCodeMessageException(call) {int x= (call); if(x != 0) {ThrowMessageException(std::string(#call).append(": ").append(strerror(x)));}}
#define errnoThrowMessageException(errnoCode, message) throw msg::ErrNoException(errnoCode, message, __FILE__, __LINE__)
#define errnoCodeThrowMessageException(errnoCode, message) if(0 != errnoCode) {errnoThrowMessageException(errno, message);} else noop()
#define errnoAssertMessageException(condition) if((condition)) {errnoCodeThrowMessageException(errno, #condition);} else noop()
#define errnoAssertPositiveMessageException(call) if( (call) < 0) {errnoCodeThrowMessageException(errno, #call);} else noop()
#define errnoNULLAssertMessageException(call) if( NULL == (call) ) {errnoThrowMessageException(errno, #call);} else noop()

template <bool B> inline void STATIC_ASSERT_IMPL() {
	char STATIC_ASSERT_FAILURE[B] = {0};
	void *x[]= {&x, &STATIC_ASSERT_FAILURE};
}
#define compileTimeAssert(B) STATIC_ASSERT_IMPL <B>()

namespace msg {

	class Exception : public std::exception {
	public:
		Exception(const char *message, const char *file= NULL, int line= 0) throw();
		Exception(const std::string &message, const char *file= NULL, int line= 0) throw();
		Exception(const Exception &other);
		Exception &operator=(const Exception &other);
		virtual ~Exception() throw();
		virtual const char* what() const throw();
	private:
		std::string	*_message;

		template<class S>
		std::string *_init(S message, const char *file, int line);
	};

	class ErrNoException : public Exception {
	public:
		ErrNoException(int errnoCode, const char *message, const char *file= NULL, int line= 0) throw();
		ErrNoException(const char *message, const char *file= NULL, int line= 0) throw();
		virtual ~ErrNoException() throw();
		virtual const char* what() const throw();
	private:
		int	_errno;
	};

	inline Exception::Exception (const char *message, const char *file, int line) throw()
		:_message(_init(message, file, line)) {
	}
	inline Exception::Exception(const std::string &message, const char *file, int line) throw()
		:_message(_init(message, file, line)) {
	}
	inline Exception::Exception(const Exception &other)
		:exception(), _message(other._message) {
		if(NULL != _message) {
			try {
				_message= new std::string(*_message);
			} catch(const std::exception&) {
			}
		}
	}
	inline Exception &Exception::operator=(const Exception &other) {
		if(this != &other) {
			delete _message;
			if(NULL != other._message) {
				try {
					_message= new std::string(*other._message);
				} catch(const std::exception&) {
				}
			}
		}
		return *this;
	}
	inline Exception::~Exception() throw() {
		delete _message;
		_message= NULL;
	}
	inline const char* Exception::what() const throw() {
		if(NULL != _message) {
			return _message->c_str();
		} else {
			return "Unable to allocate message string at exception throw!";
		}
	}
	template<class S>
	inline std::string *Exception::_init(S message, const char *file, int line) {
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
			}
		} catch (const std::exception &) {
		}
		return messagePtr;
	}
	inline ErrNoException::ErrNoException(int errnoCode, const char *message, const char *file, int line) throw()
		:Exception(std::string(message).append(": ").append(strerror(errnoCode)), file, line), _errno(errno) {
	}
	inline ErrNoException::ErrNoException(const char *message, const char *file, int line) throw()
		:Exception(std::string(message).append(strerror(errno)), file, line), _errno(errno) {
	}
	inline ErrNoException::~ErrNoException() throw() {
		_errno= 0;
	}
	inline const char* ErrNoException::what() const throw() {
		return Exception::what();
	}
};

#endif // __MessageException_h__
