#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include <sys/socket.h>
#include <string>
#include "Buffer.h"
#include "Address.h"

namespace net {
	
	/** Abstraction of a socket.
	*/
	class SocketGeneric {
		public:
			void connect(const Address &address);
			size_t read(size_t bytes, Buffer &buffer);
			size_t write(const Buffer &buffer);
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
		errnoAssertPositiveMessageException(::connection(_socket, address, address));
	}
	inline size_t SocketGeneric::read(size_t bytes, Buffer &buffer) {
		ssize_t	amount;
		
		errnoAssertPositiveMessageException(amount= ::read(_socket, buffer.start(), buffer.size()));
		return amount;
	}
	inline size_t SocketGeneric::write(const Buffer &buffer) {
		ssize_t	amount;
		
		errnoAssertPositiveMessageException(amount= ::write(_socket, buffer.start(), buffer.size()));
		return amount;
	}
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
		:_socket(::socket(domain, type, protocol) {
		errnoAssertPositiveMessageException(_socket);
	}
	inline SocketGeneric::~SocketGeneric() {
		if(-1 != socket) {
			try	{
				close();
			} catch(const std::exception&) {
			}
		}
	}
	inline void SocketGeneric::assign(int descriptor) {
		_socket= descriptor;
	}

}

#endif // __SocketGeneric_h__
