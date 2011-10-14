#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "SocketGeneric.h"
#include "Socket.h"

namespace net {

	class SocketServer : public SocketGeneric {
		public:
			SocketServer(int domain= AF_INET, int type= SOCK_STREAM, int protocol= 0);
			~SocketServer();
			void bind(Address &address);
			void listen(int backlog);
			void accept(Address &address, Socket &remote);
	};

	inline SocketServer::SocketServer(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {
	}
	inline SocketServer::~SocketServer() {}
	inline void SocketServer::bind(Address &address) {
		errnoAssertPositiveMessageException(::bind(_socket, address, address));
	}
	inline void SocketServer::listen(int backlog) {
		errnoAssertPositiveMessageException(::listen(_socket, backlog));
	}
	inline socklen_t SocketServer::accept(Address &address, Socket &remote) {
		socklen_t	size= remote;
		
		errnoAssertPositiveMessageException(::accept(_socket, remote, &size));
		return size;
	}
}

#endif // __SocketServer_h__
