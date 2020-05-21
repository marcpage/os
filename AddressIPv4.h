#ifndef __AddressIPv4_h__
#define __AddressIPv4_h__

#include "os/Address.h"
#include "os/Exception.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

namespace net {

/** An IPv4 Internet Address (AF_INET, sockaddr_in).
        @see http://www.linuxhowtos.org/C_C++/socket.htm
        @see http://uw714doc.sco.com/en/SDK_netapi/sockD.PortIPv6examples.html
*/
class AddressIPv4 : public Address {
public:
  enum IPv4Size { Size = sizeof(sockaddr_in) };
  enum IPv4Family { Family = AF_INET };
  AddressIPv4(const struct sockaddr *address, socklen_t size);
  /// @brief Initializes the address with the port and address
  AddressIPv4(in_port_t port = 0, u_int32_t address = INADDR_ANY);
  /// @brief Initializes the address with the port and named address
  AddressIPv4(const std::string &address, in_port_t port);
  virtual ~AddressIPv4();
  /// @brief Gets the address of the the sockaddr_in structure
  struct sockaddr *get() override;
  /// @brief Gets the size of the sockaddr_in structure
  socklen_t size() const override;
  /// @brief AF_INET
  sa_family_t family() const override;

private:
  /// @brief The IPv4 AF_INET structure
  struct sockaddr_in _address;
};

/// @todo Test
inline AddressIPv4::AddressIPv4(const struct sockaddr *address, socklen_t size)
    : Address(), _address() {
  AssertMessageException(NULL != address);
  printf("size=%d Size=%d\n", size, Size);
  printf("family=%d Family=%d\n",
         reinterpret_cast<const struct sockaddr *>(address)->sa_family, Family);
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
inline AddressIPv4::AddressIPv4(in_port_t port, u_int32_t address)
    : Address(), _address() {
  ::bzero(reinterpret_cast<char *>(&_address), size());
#if defined(__APPLE__)
  _address.sin_len = sizeof(_address);
#endif
  _address.sin_family = family();
  _address.sin_addr.s_addr = address;
  _address.sin_port = htons(port);
}
/**
        @param address	The address to listen on or connect to.
        @param port		The port to listen on or connect to.
        @todo test
*/
inline AddressIPv4::AddressIPv4(const std::string &address, in_port_t port)
    : Address(), _address() {
  struct hostent *hostaddress;

  ::bzero(reinterpret_cast<char *>(&_address), size());
  hostaddress = ::gethostbyname2(address.c_str(), family());
  if (NULL == hostaddress) {
    ThrowMessageException(
        std::string("gethostbyname2 failed: ").append(::hstrerror(h_errno)));
  }
#if defined(__APPLE__)
  _address.sin_len = sizeof(_address);
#endif
  _address.sin_family = hostaddress->h_addrtype;
  _address.sin_port = htons(port);
  ::bcopy(reinterpret_cast<char *>(hostaddress->h_addr),
          reinterpret_cast<char *>(&_address.sin_addr.s_addr),
          hostaddress->h_length);
}
inline AddressIPv4::~AddressIPv4() {}
/** @return The address of the sockaddr_in structure. */
inline struct sockaddr *AddressIPv4::get() {
  return reinterpret_cast<struct sockaddr *>(&_address);
}
/** @return The size of the sockaddr_in structure. */
inline socklen_t AddressIPv4::size() const { return sizeof(_address); }
/** @return AF_INET */
inline sa_family_t AddressIPv4::family() const { return AF_INET; }

} // namespace net

#endif // __AddressIPv4_h__
