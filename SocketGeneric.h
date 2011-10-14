#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include <sys/socket.h>
#include <string>
#include "Buffer.h"
#include "Address.h"
#include "Exception.h"

namespace net {

	/** Abstraction of a socket.
	*/
	class SocketGeneric {
		public:
			int descriptor();
			void close();
		protected:
			int	_socket;
			SocketGeneric();
			SocketGeneric(int domain, int type, int protocol);
			virtual ~SocketGeneric();
			void assign(int descriptor);
			friend class SocketServer;
	};

	inline int SocketGeneric::descriptor() {
		return _socket;
	}
	inline void SocketGeneric::close() {
		errnoAssertPositiveMessageException(::close(_socket));
	}
	inline SocketGeneric::SocketGeneric()
		:_socket(-1) {
	}
	inline SocketGeneric::SocketGeneric(int domain, int type, int protocol)
		:_socket(::socket(domain, type, protocol)) {
		errnoAssertPositiveMessageException(_socket);
	}
	inline SocketGeneric::~SocketGeneric() {
		if(-1 != _socket) {
			try	{
				close();
			} catch(const std::exception&) {
			}
		}
	}
	inline void SocketGeneric::assign(int socketDescriptor) {
		_socket= socketDescriptor;
	}

}

#endif // __SocketGeneric_h__
