#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "SocketGeneric.h"
#include "Socket.h"

namespace net {

	class SocketServer : public SocketGeneric {
		public:
			SocketServer(int domain, int type= SOCK_STREAM, int protocol= 0);
			virtual ~SocketServer();
			void bind(Address &address);
			void listen(int backlog);
			void accept(Address &address, Socket &remote);
	};

	inline SocketServer::SocketServer(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {
	}
	inline SocketServer::~SocketServer() {}
	inline void SocketServer::bind(Address &address) {
		errnoAssertPositiveMessageException(::bind(_socket, address, address.size()));
	}
	inline void SocketServer::listen(int backlog) {
		errnoAssertPositiveMessageException(::listen(_socket, backlog));
	}
	inline void SocketServer::accept(Address &address, Socket &remote) {
		socklen_t	size= address.size();
		int			socketDescriptor;

		errnoAssertPositiveMessageException(socketDescriptor= ::accept(_socket, address.get(), &size));
		remote.assign(socketDescriptor);
	}
}

#endif // __SocketServer_h__
