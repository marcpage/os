#ifndef __Socket_h__
#define __Socket_h__

#include "SocketGeneric.h"
#include "Buffer.h"

namespace net {

	/** A standard socket that you expect to read/write. */
	class Socket : public SocketGeneric {
		public:
			/** @todo Document */
			enum RoutingOptions {
				Route,
				BypassRouting
			};
			/** @todo Document */
			enum OutOfBandDataOptions {
				IgnoreOutOfBand,
				ProcessOutOfBand
			};
			/** Invalid socket. */
			Socket();
			/** Creates a new socket */
			Socket(int domain, int type= SOCK_STREAM, int protocol= 0);
			/** nothing beyond super class behavior. */
			virtual ~Socket();
			/** Connect to a given address. */
			void connect(Address &address);
			/** enables permission to transmit broadcast messages */
			void broadcast(bool cast=true);
			/** Read bytes into a buffer from the socket. */
			size_t read(Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
			/** Write bytes from a buffer to the socket. */
			size_t write(const Buffer &buffer, size_t bytes= static_cast<size_t>(-1));
			/** @todo Document */
			size_t sendto(const Address &address, const Buffer &buffer, size_t bytes=static_cast<size_t>(-1), RoutingOptions route=Route, OutOfBandDataOptions outOfBand=IgnoreOutOfBand);
	};

	inline Socket::Socket()
		:SocketGeneric() {}
	/**
		@param domain	The domain or family <code>Address.family()</code>
		@param type		The type of socket (ie SOCK_STREAM)
		@param protocol	The socket protocol (usually 0?)
	*/
	inline Socket::Socket(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {}
	inline Socket::~Socket() {}
	/** @param address	The address to connect to. */
	inline void Socket::connect(Address &address) {
		ErrnoOnNegative(::connect(_socket, address, address.size()));
	}
	/** @todo Document*/
	inline void Socket::broadcast(bool cast) {
		setOption(SO_BROADCAST, cast);
	}
	/**
		@param buffer	The buffer to fill
		@param bytes	The number of bytes to but in the buffer, or -1 for buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
		@todo	Figure out when we are end of stream and return -1
	*/
	inline size_t Socket::read(Buffer &buffer, size_t bytes) {
		ssize_t	amount;
		size_t	toRead= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		if(bytes > buffer.size()) {
			bytes= buffer.size();
		}
		ErrnoOnNegative(amount= ::read(_socket, buffer.start(), toRead));
		return amount;
	}
	/**
		@param buffer	The buffer to send
		@param bytes	The number of bytes in the buffer to send, or -1 for the entire buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
	*/
	inline size_t Socket::write(const Buffer &buffer, size_t bytes) {
		ssize_t	amount;
		const size_t	toWrite= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;

		if(bytes > buffer.size()) {
			bytes= buffer.size();
		}
		ErrnoOnNegative(amount= ::write(_socket, buffer.start(), toWrite));
		return amount;
	}
	/**
		@todo Document
	*/
	inline size_t Socket::sendto(const Address &address, const Buffer &buffer, size_t bytes, RoutingOptions route, OutOfBandDataOptions outOfBand) {
		ssize_t			amount;
		const size_t	toSend= bytes == static_cast<size_t>(-1) ? buffer.size() : bytes;
		const int		flags= (BypassRouting == route ? MSG_DONTROUTE : 0) | (IgnoreOutOfBand == outOfBand ? MSG_OOB : 0);

		ErrnoOnNegative(amount= ::sendto(_socket, buffer.start(), toSend, flags, address.get(), address.size()));
		return amount;
	}

}

#endif // __Socket_h__
