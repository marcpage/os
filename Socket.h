#ifndef __Socket_h__
#define __Socket_h__

#include "SocketGeneric.h"

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

namespace net {

	/** A standard socket that you expect to read/write. */
	class Socket : public SocketGeneric {
		public:
			/** Invalid socket. */
			Socket();
			/** Creates a new socket */
			Socket(int domain, int type= SOCK_STREAM, int protocol= 0);
			/** nothing beyond super class behavior. */
			virtual ~Socket();
			/** Connect to a given address. */
			void connect(Address &address);
			/** Read bytes into a buffer from the socket. */
			size_t read(Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
			/** Write bytes from a buffer to the socket. */
			size_t write(const Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
	};

	inline Socket::Socket()
		:SocketGeneric() {trace_scope}
	/**
		@param domain	The domain or family <code>Address.family()</code>
		@param type		The type of socket (ie SOCK_STREAM)
		@param protocol	The socket protocol (usually 0?)
	*/
	inline Socket::Socket(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {trace_scope}
	inline Socket::~Socket() {trace_scope}
	/** @param address	The address to connect to. */
	inline void Socket::connect(Address &address) {trace_scope
		errnoAssertPositiveMessageException(::connect(_socket, address, address.size()));
	}
	/**
		@param buffer	The buffer to fill
		@param bytes	The number of bytes to but in the buffer, or -1 for buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
		@todo	Figure out when we are end of stream and return -1
	*/
	inline size_t Socket::read(Buffer &buffer, size_t bytes) {trace_scope
		ssize_t	amount;
		size_t	toRead= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		if(bytes > buffer.size()) {
			bytes= buffer.size();
		}
		errnoAssertPositiveMessageException(amount= ::read(_socket, buffer.start(), toRead));
		return amount;
	}
	/**
		@param buffer	The buffer to send
		@param bytes	The number of bytes in the buffer to send, or -1 for the entire buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
	*/
	inline size_t Socket::write(const Buffer &buffer, size_t bytes) {trace_scope
		ssize_t	amount;
		size_t	toWrite= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		if(bytes > buffer.size()) {
			bytes= buffer.size();
		}
		errnoAssertPositiveMessageException(amount= ::write(_socket, buffer.start(), toWrite));
		return amount;
	}
}

#endif // __Socket_h__
