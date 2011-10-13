#ifndef __Socket_h__
#define __Socket_h__

#incude "SocketGeneric.h"

namespace net {

	class Socket : public SocketGeneric {
		public:
			enum Descriptor {
				descriptor
			};
			Socket();
			Socket(int domain= AF_INET, int type= SOCK_STREAM, int protocol= 0);
			~Socket();
			void connect(Address &address);
	};
}

#endif // __Socket_h__
