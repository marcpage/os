#ifndef __MessageException_h__
#define __MessageException_h__

/** @file Exception.h */
#include "os/Backtrace.h"
#include <exception>
#include <string>

namespace msg {
inline void noop() {} ///< Used for if() else corrrectness with macros
} // namespace msg

// NOTE: __func__ is C99 standard

/// Throws an exception without condition
#define ThrowMessageException(message)                                         \
  throw msg::Exception((message), __FILE__, __LINE__, __func__)
/// Throws an exception if an expression is nullptr
#define ThrowMessageExceptionIfNULL(variable)                                  \
  if (nullptr == (variable)) {                                                 \
    ThrowMessageException(std::string(#variable).append(" == nullptr"));       \
  } else                                                                       \
    msg::noop()
/// Throws and exception if an asserted condition is false
#define AssertMessageException(condition)                                      \
  if (!(condition)) {                                                          \
    ThrowMessageException(#condition);                                         \
  } else                                                                       \
    msg::noop()
/// Intended for use in wrapping a function that returns an int, of which zero
/// means no error, and any other value is an error. Only throws on error.
#define AssertCodeMessageException(call)                                       \
  {                                                                            \
    int x = (call);                                                            \
    if (x != 0) {                                                              \
      ThrowMessageException(                                                   \
          std::string(#call).append(": ").append(strerror(x)));                \
    }                                                                          \
  }

namespace msg {
/// Helper function to break at compile time on an assertion failure.
template <bool B> inline void STATIC_ASSERT_IMPL() {
  char STATIC_ASSERT_FAILURE[B] = {0};
  void *x[] = {&x, &STATIC_ASSERT_FAILURE};
}
} // namespace msg

/// Assert B at compile time (break the compile if assertion fails).
#define compileTimeAssert(B) msg::STATIC_ASSERT_IMPL<B>()

/** Contains Exception and helpers.
 */
namespace msg {

/** A general exception the maintains the file and line numbers.
 */
class Exception : public std::exception {
public:
  /// Get a message
  explicit Exception(const char *message, const char *file = nullptr,
                     int line = 0, const char *function = nullptr) throw();
  /// Get a message
  explicit Exception(const std::string &message, const char *file = nullptr,
                     int line = 0, const char *function = nullptr) throw();
  /// Copy constructor
  Exception(const Exception &other);
  /// assignment operator
  Exception &operator=(const Exception &other);
  /// destructs _message
  virtual ~Exception() throw();
  /// gets the message
  virtual const char *what() const throw();

private:
  std::string *_message; ///< The message about why the exception was thrown.

  /// Helper function to initialize from the various constructors.
  template <class S>
  std::string *_init(S message, const char *file, int line,
                     const char *function);
};

/**
        @param message	The reason for the exception
        @param file		Set to __FILE__
        @param line		Set to __LINE__
*/
inline Exception::Exception(const char *message, const char *file, int line,
                            const char *function) throw()
    : _message(_init(message, file, line, function)) {}
/**
        @param message	The reason for the exception
        @param file		Set to __FILE__
        @param line		Set to __LINE__
*/
inline Exception::Exception(const std::string &message, const char *file,
                            int line, const char *function) throw()
    : _message(_init(message, file, line, function)) {}
/**
        @param other	The exception to copy.
*/
inline Exception::Exception(const Exception &other)
    : exception(), _message(other._message) {
  if (nullptr != _message) {
    try {
      _message = new std::string(*_message);
    } catch (const std::exception &) {
      _message = nullptr;
    }
  }
}
/**
        @param other	The exception to copy.
*/
inline Exception &Exception::operator=(const Exception &other) {
  if (this != &other) {
    if (nullptr != other._message) {
      try {
        if (nullptr != _message) {
          _message->assign(*other._message);
        } else {
          _message = new std::string(*other._message);
        }
      } catch (const std::exception &) {
      }
    } else {
      delete _message;
      _message = nullptr;
    }
  }
  return *this;
}
/** Deletes _message.
 */
inline Exception::~Exception() throw() {
  delete _message;
  _message = nullptr;
}
/**
        @return	The reason why the exception was thrown.
*/
inline const char *Exception::what() const throw() {
  if (nullptr != _message) {
    return _message->c_str();
  } else {
    return "Unable to allocate " // not tested
           "message string at exception throw!";
  }
}
/** Allows for the same code to be used to initialize, regardless of the string
   type.
        @tparam S		The string type, either const char * or
   std::string
        @param message	The reason for the exception
        @param file		Set to __FILE__
        @param line		Set to __LINE__
        @return			A std::string of the complete message.
*/
template <class S>
inline std::string *Exception::_init(S message, const char *file, int line,
                                     const char *function) {
  std::string *messagePtr = nullptr;

  try {
    trace::StringList stack;

    trace::stack(stack);
    messagePtr = new std::string(message);
    if (nullptr != file) {
      messagePtr->append(" File: ").append(file).append(" Line: ").append(
          std::to_string(line));
      if (nullptr != function) {
        messagePtr->append(" Function: ").append(function);
      }
      for (auto i : stack) {
        messagePtr->append("\n" + i);
      }
    }
  } catch (const std::exception &) {
  }
  return messagePtr;
}
}; // namespace msg

#endif // __MessageException_h__
