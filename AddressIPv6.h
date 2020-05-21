#ifndef __AddressIPv6_h__
#define __AddressIPv6_h__

#include "Address.h"
#include "Exception.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

namespace net {

/** An IPv6 Internet Address (AF_INET6, sockaddr_in6).
        @see http://uw714doc.sco.com/en/SDK_netapi/sockD.PortIPv6examples.html
*/
class AddressIPv6 : public Address {
public:
  enum IPv6Size { Size = sizeof(sockaddr_in6) };
  enum IPv6Family { Family = AF_INET6 };
  AddressIPv6(const void *address, socklen_t size);
  /// @brief Initializes the address with the port and address
  AddressIPv6(in_port_t port = 0, const struct in6_addr &address = in6addr_any);
  /// @brief Initializes the address with the port and named address
  AddressIPv6(const std::string &address, in_port_t port);
  virtual ~AddressIPv6();
  /// @brief Gets the address of the the sockaddr_in6 structure
  struct sockaddr *get() override;
  /// @brief Gets the size of the sockaddr_in6 structure
  socklen_t size() const override;
  /// @brief AF_INET6
  sa_family_t family() const override;

private:
  /// @brief The IPv6 AF_INET structure
  struct sockaddr_in6 _address;
};

/// @todo Test
inline AddressIPv6::AddressIPv6(const void *address, socklen_t size)
    : Address(), _address() {
  AssertMessageException(NULL != address);
  AssertMessageException(Size == size);
  AssertMessageException(
      Family == reinterpret_cast<const struct sockaddr *>(address)->sa_family);
  ::memcpy(&_address, address, Size);
}
/**
        @param port		The port to listen on or connect to.
        @param address	The address to listen on or connect to. Defaults to
   listen on all.
*/
inline AddressIPv6::AddressIPv6(in_port_t port, const struct in6_addr &address)
    : Address(), _address() {
  ::bzero(reinterpret_cast<char *>(&_address), size());
#if defined(__APPLE__)
  _address.sin6_len = sizeof(_address);
#endif
  _address.sin6_family = family();
  _address.sin6_addr = address;
  _address.sin6_port = htons(port);
}
/**
        @param address	The address to listen on or connect to.
        @param port		The port to listen on or connect to.
        @todo test
*/
inline AddressIPv6::AddressIPv6(const std::string &address, in_port_t port)
    : Address(), _address() {
  struct hostent *hostaddress;

  ::bzero(reinterpret_cast<char *>(&_address), size());
  hostaddress = ::gethostbyname2(address.c_str(), family());
  if (NULL == hostaddress) {
    ThrowMessageException(
        std::string("gethostbyname2 failed: ").append(::hstrerror(h_errno)));
  }
#if defined(__APPLE__)
  _address.sin6_len = sizeof(_address);
#endif
  _address.sin6_family = hostaddress->h_addrtype;
  _address.sin6_port = htons(port);
  ::bcopy(reinterpret_cast<char *>(hostaddress->h_addr),
          reinterpret_cast<char *>(&_address.sin6_addr), hostaddress->h_length);
}
inline AddressIPv6::~AddressIPv6() {}
/** @return The address of the sockaddr_in6 structure. */
inline struct sockaddr *AddressIPv6::get() {
  return reinterpret_cast<struct sockaddr *>(&_address);
}
/** @return The size of the sockaddr_in6 structure. */
inline socklen_t AddressIPv6::size() const { return sizeof(_address); }
/** @return AF_INET6 */
inline sa_family_t AddressIPv6::family() const { return AF_INET6; }
} // namespace net

#endif // __AddressIPv6_h__
