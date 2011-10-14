#ifndef __Socket_h__
#define __Socket_h__

#incude "SocketGeneric.h"

namespace net {

	class Socket : public SocketGeneric {
		public:
			Socket();
			Socket(int domain= AF_INET, int type= SOCK_STREAM, int protocol= 0);
			~Socket();
			void connect(Address &address);
	};

	inline Socket::Socket()
		:SocketGeneric() {
	}
	inline Socket::Socket(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {}
	inline Socket::~Socket() {}
	inline void Socket::connect(Address &address) {
		errnoAssertPositiveMessageException(::connection(_socket, address, address));
	}
}

#endif // __Socket_h__
