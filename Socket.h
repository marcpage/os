#ifndef __Socket_h__
#define __Socket_h__

#include "SocketGeneric.h"

namespace net {

	class Socket : public SocketGeneric {
		public:
			Socket();
			Socket(int domain, int type= SOCK_STREAM, int protocol= 0);
			virtual ~Socket();
			void connect(Address &address);
			size_t read(Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
			size_t write(const Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
	};

	inline Socket::Socket()
		:SocketGeneric() {
	}
	inline Socket::Socket(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {}
	inline Socket::~Socket() {}
	inline void Socket::connect(Address &address) {
		errnoAssertPositiveMessageException(::connect(_socket, address, address.size()));
	}
	inline size_t Socket::read(Buffer &buffer, size_t bytes) {
		ssize_t	amount;
		size_t	toRead= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		if(bytes > buffer.size()) {
			bytes= buffer.size();
		}
		errnoAssertPositiveMessageException(amount= ::read(_socket, buffer.start(), toRead));
		return amount;
	}
	inline size_t Socket::write(const Buffer &buffer, size_t bytes) {
		ssize_t	amount;
		size_t	toWrite= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		errnoAssertPositiveMessageException(amount= ::write(_socket, buffer.start(), toWrite));
		return amount;
	}
}

#endif // __Socket_h__
