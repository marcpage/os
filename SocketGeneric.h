#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include <sys/socket.h>
#include <string>
#include "Buffer.h"

namespace net {

	class SocketGeneric {
		public:
			class Address {
				public:
					Address() {}
					~Address() {}
					operator struct sockaddr*() {return get();}
					operator socklen_t() {return size();}
					struct sockaddr *get()=0;
					socklen_t size()=0;
			};
			void connect(const Address &address);
			size_t read(size_t bytes, Buffer &buffer);
			void write(const Buffer &buffer);
			int descriptor();
			void close();
		protected:
			int	_socket;
			SocketGeneric();
			SocketGeneric(int domain, int type, int protocol);
			~SocketGeneric();
			void assign(int descriptor);
	};

	inline void SocketGeneric::connect(const Address &address) {
		errnoAssertPositiveMessageException(socket(_socket, address, address));
	}
	inline size_t SocketGeneric::read(size_t bytes, Buffer &buffer) {
		ssize_t	amount= read(_socket, buffer.start(), buffer.size());
		return buffer;
	}
	inline void SocketGeneric::write(const Buffer &buffer) {
	}
	inline int SocketGeneric::descriptor() {
	}
	inline void SocketGeneric::close() {
	}
	inline SocketGeneric::SocketGeneric() {
	}
	inline SocketGeneric::SocketGeneric(int domain, int type, int protocol)
		:_socket(socket(domain, type, protocol) {
	}
	inline SocketGeneric::~SocketGeneric() {
	}
	inline void SocketGeneric::assign(int descriptor) {
	}

}

#endif // __SocketGeneric_h__
