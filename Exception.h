#ifndef __MessageException_h__
#define __MessageException_h__

/** @file Exception.h
	@todo add __func__ or __FUNCTION__, whichever is more appropriate and only if available
	@todo write a test file for Exception
	@todo figure out how to test errnoAssertMessageException, errnoAssertPositiveMessageException
					and errnoNULLAssertMessageException
*/
#include <string>
#include <exception>
#include <sstream>
#include <errno.h>

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) x ///< in case Tracer.h is not included
#endif

namespace msg {
	inline void noop() {} ///< Used for if() else corrrectness with macros
}

/// Throws an exception without condition
#define ThrowMessageException(message) throw msg::Exception((message), __FILE__, __LINE__)
/// Throws an exception if an expression is NULL
#define ThrowMessageExceptionIfNULL(variable) if(NULL == (variable)) {ThrowMessageException(std::string(#variable).append(" == NULL"));} else msg::noop()
/// Throws and exception if an asserted condition is false
#define AssertMessageException(condition) if(!(condition)) {ThrowMessageException(#condition);} else msg::noop()
/// Intended for use in wrapping a function that returns an int, of which zero means no error, and any other value is an error. Only throws on error.
#define AssertCodeMessageException(call) {int x= (call); if(x != 0) {ThrowMessageException(std::string(#call).append(": ").append(strerror(x)));}}
/// Inteded for use by other macros, throws an msg::ErrNoException with the given errno code.
#define errnoThrowMessageExceptionCore(errnoCode, message, File, Line) throw msg::ErrNoException(errnoCode, message, File, Line)
/// Throws a msg::ErrNoException with the given errno code.
#define errnoThrowMessageException(errnoCode, message) errnoThrowMessageExceptionCore(errnoCode, message, __FILE__, __LINE__)
/// Throws a msg::ErrNoException if the errnoCode is not 0 (ie there was an error).
#define errnoCodeThrowMessageException(errnoCode, message) if(0 != errnoCode) {errnoThrowMessageExceptionCore(errnoCode, message, __FILE__, __LINE__);} else msg::noop()
/// Throws a msg::ErrNoException with errno as the errnoCode if the assertion fails (ie the condition is false)
#define errnoAssertMessageException(condition) if(!(condition)) {errnoCodeThrowMessageException(errno, #condition);} else msg::noop()
/// Throws a msg::ErrNoException with errno as the errnoCode if the result of a call is less than zero (UNIX calls usually return -1 on error and set errno)
#define errnoAssertPositiveMessageException(call) if( (call) < 0) {errnoCodeThrowMessageException(errno, #call);} else msg::noop()
/// Throws a msg::ErrNoException with errno as the errnoCode if the result of a call is NULL
#define errnoNULLAssertMessageException(call) if( NULL == (call) ) {errnoThrowMessageExceptionCore(errno, #call, __FILE__, __LINE__);} else msg::noop()

namespace msg {
	/// Helper function to break at compile time on an assertion failure.
	template <bool B> inline void STATIC_ASSERT_IMPL() {
		char STATIC_ASSERT_FAILURE[B] = {0};
		void *x[]= {&x, &STATIC_ASSERT_FAILURE};
	}
}

/// Assert B at compile time (break the compile if assertion fails).
#define compileTimeAssert(B) msg::STATIC_ASSERT_IMPL <B>()

/** Contains Exception and helpers.
*/
namespace msg {

	/** A general exception the maintains the file and line numbers.
	*/
	class Exception : public std::exception {
	public:
		/// Get a message
		Exception(const char *message, const char *file= NULL, int line= 0) throw();
		/// Get a message
		Exception(const std::string &message, const char *file= NULL, int line= 0) throw();
		/// Copy constructor
		Exception(const Exception &other);
		/// assignment operator
		Exception &operator=(const Exception &other);
		/// destructs _message
		virtual ~Exception() throw();
		/// gets the message
		virtual const char* what() const throw();
	private:
		std::string	*_message; ///< The message about why the exception was thrown.

		/// Helper function to initialize from the various constructors.
		template<class S>
		std::string *_init(S message, const char *file, int line);
	};

	/** Handles errno specific details for UNIX functions that set errno.
	*/
	class ErrNoException : public Exception {
	public:
		/// errnoCode and message
		ErrNoException(int errnoCode, const char *message, const char *file= NULL, int line= 0) throw();
		/// Just a message, errno will be read for the code
		ErrNoException(const char *message, const char *file= NULL, int line= 0) throw();
		/// nothing special here
		virtual ~ErrNoException() throw();
		/// again, nothing special
		virtual const char* what() const throw();
	private:
		int	_errno; ///< the errono code
	};

	/**
		@param message	The reason for the exception
		@param file		Set to __FILE__
		@param line		Set to __LINE__
	*/
	inline Exception::Exception (const char *message, const char *file, int line) throw()
		:_message(_init(message, file, line)) {trace_scope
	}
	/**
		@param message	The reason for the exception
		@param file		Set to __FILE__
		@param line		Set to __LINE__
	*/
	inline Exception::Exception(const std::string &message, const char *file, int line) throw()
		:_message(_init(message, file, line)) {trace_scope
	}
	/**
		@param other	The exception to copy.
	*/
	inline Exception::Exception(const Exception &other)
		:exception(), _message(other._message) {trace_scope
		if(NULL != _message) {
			try {
				_message= new std::string(*_message);
			} catch(const std::exception&) {
			}
		}
	}
	/**
		@param other	The exception to copy.
	*/
	inline Exception &Exception::operator=(const Exception &other) {trace_scope
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
	/** Deletes _message.
	*/
	inline Exception::~Exception() throw() {trace_scope
		delete _message;
		_message= NULL;
	}
	/**
		@return	The reason why the exception was thrown.
	*/
	inline const char* Exception::what() const throw() {trace_scope
		if(NULL != _message) {
			return _message->c_str();
		} else {
			return "Unable to allocate message string at exception throw!";
		}
	}
	/** Allows for the same code to be used to initialize, regardless of the string type.
		@tparam S		The string type, either const char * or std::string
		@param message	The reason for the exception
		@param file		Set to __FILE__
		@param line		Set to __LINE__
		@return			A std::string of the complete message.
	*/
	template<class S>
	inline std::string *Exception::_init(S message, const char *file, int line) {trace_scope
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
	/** Used if you already have the errno code for some reason, errno will not be called.
		Also calls strerror to get the system string for the errono code.
		@param errnoCode	from calling errno
		@param message		The reason for the exception
		@param file			Set to __FILE__
		@param line			Set to __LINE__
	*/
	inline ErrNoException::ErrNoException(int errnoCode, const char *message, const char *file, int line) throw()
		:Exception(std::string(message).append(": ").append(strerror(errnoCode)), file, line), _errno(errnoCode) {trace_scope
	}
	/** Used if you do not have the errno code, errno will be called.
		Also calls strerror to get the system string for the errono code.
		@param message		The reason for the exception
		@param file			Set to __FILE__
		@param line			Set to __LINE__
	*/
	inline ErrNoException::ErrNoException(const char *message, const char *file, int line) throw()
		:Exception(std::string(message).append(strerror(errno)), file, line), _errno(errno) {trace_scope
	}
	/** Cleans up _errno.
	*/
	inline ErrNoException::~ErrNoException() throw() {trace_scope
		_errno= 0;
	}
	/** Just call Exception's what().
	*/
	inline const char* ErrNoException::what() const throw() {trace_scope
		return Exception::what();
	}
};

#endif // __MessageException_h__
