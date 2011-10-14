#ifndef __AddressIPv4_h__
#define __AddressIPv4_h__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include "Address.h"

namespace net {

	/** Reference: http://www.linuxhowtos.org/C_C++/socket.htm
	*/
	class AddressIPv4 : public Address {
		public:
			AddressIPv4(u_int32_t address, in_port_t port, sa_family_t family= AF_INET);
			AddressIPv4(const std::string &address, in_port_t port, sa_family_t family= AF_INET);
			~AddressIPv4();
			struct sockaddr *get();
			socklen_t size();
		private:
			struct sockaddr_in	_address;
	};

	inline AddressIPv4::AddressIPv4(uint32_t address, in_port_t port, sa_family_t family= AF_INET)
		:Address(), _address() {
		::bzero(reinterpret_cast<char*>(&_address), size());
		_address.sin_addr.s_addr= INADDR_ANY;
		_address.sin_port = ::htons(port);
	}
	inline AddressIPv4::AddressIPv4(const std::string &address, in_port_t port, sa_family_t family)
		:Address(), _address() {
		struct hostent	*address;
		
		::bzero(reinterpret_cast<char*>(&_address), size());
		address= ::gethostbyname2(address.c_str(), family);
		if(NULL == address) {
			ThrowMessageException(std::string("gethostbyname2 failed: ").append(::hstrerror(::h_errno)));
		}
		::bcopy(reinterpret_cast<char*>(address->h_addr),
				reinterpret_cast<char*>(&_address.sin_addr.s_addr),
				address->h_length
		);
		_address.sin_port = ::htons(port);
	}
	inline AddressIPv4::~AddressIPv4() {}
	inline struct sockaddr *AddressIPv4::get() {
		return reinterpret_cast<sockaddr*>(_address);
	}
	inline socklen_t AddressIPv4::size() {
		return sizeof(_address);
	}

}

#endif // __AddressIPv4_h__
