#ifndef __AddressIPv6_h__
#define __AddressIPv6_h__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include "Address.h"

namespace net {

	/** Reference: http://uw714doc.sco.com/en/SDK_netapi/sockD.PortIPv6examples.html
	*/
	class AddressIPv6 : public Address {
		public:
			AddressIPv6(in_port_t port= 0, const struct in6_addr &address= in6addr_any);
			AddressIPv6(const std::string &address, in_port_t port);
			virtual ~AddressIPv6();
			virtual struct sockaddr *get();
			virtual socklen_t size() const;
			virtual sa_family_t family() const;
		private:
			struct sockaddr_in6	_address;
	};

	inline AddressIPv6::AddressIPv6(in_port_t port, const struct in6_addr &address)
		:Address(), _address() {
		::bzero(reinterpret_cast<char*>(&_address), size());
		_address.sin6_len= sizeof(_address);
		_address.sin6_family= family();
		_address.sin6_addr= address;
		_address.sin6_port = htons(port);
	}
	inline AddressIPv6::AddressIPv6(const std::string &address, in_port_t port)
		:Address(), _address() {
		struct hostent	*hostaddress;

		::bzero(reinterpret_cast<char*>(&_address), size());
		hostaddress= ::gethostbyname2(address.c_str(), family());
		if(NULL == hostaddress) {
			ThrowMessageException(std::string("gethostbyname2 failed: ").append(::hstrerror(::h_errno)));
		}
		_address.sin6_len= sizeof(_address);
		_address.sin6_family= hostaddress->h_addrtype;
		_address.sin6_port = htons(port);
		::bcopy(reinterpret_cast<char*>(hostaddress->h_addr),
				reinterpret_cast<char*>(&_address.sin6_addr),
				hostaddress->h_length
		);
	}
	inline AddressIPv6::~AddressIPv6() {}
	inline struct sockaddr *AddressIPv6::get() {
		return reinterpret_cast<struct sockaddr*>(&_address);
	}
	inline socklen_t AddressIPv6::size() const {
		return sizeof(_address);
	}
	inline sa_family_t AddressIPv6::family() const {
		return AF_INET6;
	}
}

#endif // __AddressIPv6_h__
