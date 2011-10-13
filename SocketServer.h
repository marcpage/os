#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "SocketGeneric.h"

namespace net {

	class SocketServer : public SocketGeneric {
		public:
			SocketServer(int domain= AF_INET, int type= SOCK_STREAM, int protocol= 0);
			~SocketServer();
			void bind(Address &address);
			void listen(int backlog);
			void accept(Address &address, Socket &remote);
	};
}

#endif // __SocketServer_h__
