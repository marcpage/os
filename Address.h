#ifndef __Address_h__
#define __Address_h__

#include "os/Exception.h"
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

/** Networking related classes.
        @todo Test!
*/
namespace net {

#define GAIMessageThrow(condition)                                             \
  net::GetAddressInfoException::_throw(condition, #condition, __FILE__,        \
                                       __LINE__)

class GetAddressInfoException : public msg::Exception {
public:
  GetAddressInfoException(int value, const char *name, const char *file = NULL,
                          int line = 0) throw();
  GetAddressInfoException(const std::string &message, int value,
                          const char *name, const char *file, int line) throw();
  GetAddressInfoException(const GetAddressInfoException &other);
  GetAddressInfoException &operator=(const GetAddressInfoException &other);
  virtual ~GetAddressInfoException() throw();
  virtual const char *name() const;
  int code() const throw();
  static void _throw(int value, const std::string &message, const char *file,
                     int line);
  static void _noop();

private:
  int _value;

  template <class S> std::string _init(S message, const char *name, int value);
};

#define GAIException(GAI_name)                                                 \
  class GAI_name##_GAI : public GetAddressInfoException {                      \
  public:                                                                      \
    const char *name() const override { return #GAI_name; }                    \
    explicit GAI_name##_GAI(const char *file = NULL, int line = 0) throw()     \
        : GetAddressInfoException(GAI_name, #GAI_name, file, line) {}          \
    explicit GAI_name##_GAI(const std::string &message,                        \
                            const char *file = NULL, int line = 0) throw()     \
        : GetAddressInfoException(message, GAI_name, #GAI_name, file, line) {} \
  }

GAIException(EAI_AGAIN);
GAIException(EAI_BADFLAGS);
#if defined(EAI_BADHINTS)
GAIException(EAI_BADHINTS);
#endif
#if defined(EAI_PROTOCOL)
GAIException(EAI_PROTOCOL);
#endif
GAIException(EAI_FAIL);
GAIException(EAI_FAMILY);
GAIException(EAI_MEMORY);
GAIException(EAI_NONAME);
GAIException(EAI_OVERFLOW);
GAIException(EAI_SERVICE);
GAIException(EAI_SOCKTYPE);
GAIException(EAI_SYSTEM);

#undef GAIException

/** Abstraction of a Network Address */
class Address {
public:
  enum Qualified { FullyQualified, UnqualifiedLocal };
  enum Format { Numeric, Name };
  enum Required { NameRequired, NameIfAvailable };
  /** noop.*/
  virtual ~Address() {}
  /** Allow an <code>Address</code> to be passed as a sockaddr*.
          @return same as <code>get()</code>
  */
  operator struct sockaddr *() { return get(); }
  /** @todo Document */
  std::string name(Qualified qualified = FullyQualified, Format format = Name,
                   Required required = NameIfAvailable) const;
  /** Get the address for when we are a const instance.
          @return the const sockaddr address
  */
  const struct sockaddr *get() const {
    return const_cast<Address *>(this)->get();
  }
  /** @return the address of the appropriate address structure. */
  virtual struct sockaddr *get() = 0;
  /** @return the size of the socket address structure returned by
   * <code>get()</code>. */
  virtual socklen_t size() const = 0;
  /** @return the domain or address family used. */
  virtual sa_family_t family() const = 0;
};

inline GetAddressInfoException::GetAddressInfoException(int value,
                                                        const char *name,
                                                        const char *file,
                                                        int line) throw()
    : msg::Exception(_init("", name, value), file, line), _value(value) {}
inline GetAddressInfoException::GetAddressInfoException(
    const std::string &message, int value, const char *name, const char *file,
    int line) throw()
    : msg::Exception(_init(message, name, value), file, line), _value(value) {}
inline GetAddressInfoException::GetAddressInfoException(
    const GetAddressInfoException &other)
    : msg::Exception(other), _value(other._value) {}
inline GetAddressInfoException &
GetAddressInfoException::operator=(const GetAddressInfoException &other) {
  *reinterpret_cast<msg::Exception *>(this) = other;
  _value = other._value;
  return *this;
}
inline GetAddressInfoException::~GetAddressInfoException() throw() {}
inline const char *GetAddressInfoException::name() const { return "[Unknown]"; }
inline int GetAddressInfoException::code() const throw() { return _value; }
inline void GetAddressInfoException::_throw(int value,
                                            const std::string &message,
                                            const char *file, int line) {
  if (0 != value) {
    switch (value) {
#define GAICaseClass(name)                                                     \
  case name:                                                                   \
    throw name##_GAI(message, file, line)
      GAICaseClass(EAI_AGAIN);
      GAICaseClass(EAI_BADFLAGS);
#if defined(EAI_BADHINTS)
      GAICaseClass(EAI_BADHINTS);
#endif
#if defined(EAI_PROTOCOL)
      GAICaseClass(EAI_PROTOCOL);
#endif
      GAICaseClass(EAI_FAIL);
      GAICaseClass(EAI_FAMILY);
      GAICaseClass(EAI_MEMORY);
      GAICaseClass(EAI_NONAME);
      GAICaseClass(EAI_OVERFLOW);
      GAICaseClass(EAI_SERVICE);
      GAICaseClass(EAI_SOCKTYPE);
      GAICaseClass(EAI_SYSTEM);
#undef GAICaseClass
    default:
      throw GetAddressInfoException(message, value, "[Unknown]", file, line);
    }
  }
}
inline void GetAddressInfoException::_noop() {}
template <class S>
inline std::string GetAddressInfoException::_init(S message, const char *name,
                                                  int value) {
  return std::string("[") + name + "] (" + std::to_string(value) +
         "): " + gai_strerror(value) + ": " + std::string(message);
}

inline std::string Address::name(Qualified qualified, Format format,
                                 Required required) const {
  std::string dnsName(NI_MAXHOST, '\0');
  const int flags = (UnqualifiedLocal == qualified ? NI_NOFQDN : 0) |
                    (Numeric == format ? NI_NUMERICHOST : 0) |
                    (NameRequired == required ? NI_NAMEREQD : 0);

  GAIMessageThrow(::getnameinfo(get(), size(),
                                const_cast<char *>(dnsName.data()),
                                dnsName.size(), NULL, 0, flags));
  return dnsName;
}
} // namespace net

#endif // __Address_h__
