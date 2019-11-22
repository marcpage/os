#ifndef __Socket_h__
#define __Socket_h__

#include "SocketGeneric.h"

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
			/** Read bytes into a buffer from the socket. */
			std::string read(size_t bytes);
			/** Read bytes into a buffer from the socket. */
			std::string &read(size_t bytes, std::string &buffer);
			/** Write bytes from a buffer to the socket. */
			size_t write(const std::string &buffer, size_t bytes= static_cast<size_t>(-1), size_t offset= 0);
			/** @todo Document */
			size_t sendTo(const Address &address, const std::string &buffer, size_t bytes=static_cast<size_t>(-1), size_t offset= 0, RoutingOptions route=Route, OutOfBandDataOptions outOfBand=IgnoreOutOfBand);
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
	/**
		@param bytes	The number of bytes to but in the buffer, or -1 for buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
		@todo	Figure out when we are end of stream and return -1
	*/
	inline std::string Socket::read(size_t bytes) {
		std::string buffer;
		return read(bytes, buffer);
	}
	/**
		@param bytes	The number of bytes to but in the buffer, or -1 for buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
		@todo	Figure out when we are end of stream and return -1
	*/
	inline std::string &Socket::read(size_t bytes, std::string &buffer) {
		ssize_t		amount;

		buffer.assign(bytes, '\0');
		ErrnoOnNegative(amount= ::read(_socket, const_cast<char*>(buffer.data()), bytes));
		buffer.erase(amount);
		return buffer;
	}
	/**
		@param buffer	The buffer to send
		@param bytes	The number of bytes in the buffer to send, or -1 for the entire buffer max.
							If <code>bytes</code> is greater than the buffer size, the
							buffer max will be used.
		@param offset	The offset in the buffer to start writing bytes from.
	*/
	inline size_t Socket::write(const std::string &buffer, size_t bytes, size_t offset) {
		ssize_t	amount;
		size_t	toWrite= bytes == static_cast<size_t>(-1) ? buffer.size() - offset : bytes;

		if(offset + toWrite > buffer.size()) {
			toWrite= buffer.size() - offset;
		}
		ErrnoOnNegative(amount= ::write(_socket, buffer.data() + offset, toWrite));
		return amount;
	}
	/**
		@todo Document
	*/
	inline size_t Socket::sendTo(const Address &address, const std::string &buffer, size_t bytes, size_t offset, RoutingOptions route, OutOfBandDataOptions outOfBand) {
		ssize_t			amount;
		const size_t	toSend= bytes == static_cast<size_t>(-1) ? buffer.size() - offset : bytes;
		const int		flags= (BypassRouting == route ? MSG_DONTROUTE : 0) | (IgnoreOutOfBand == outOfBand ? MSG_OOB : 0);

		ErrnoOnNegative(amount= ::sendto(_socket, buffer.data() + offset, toSend, flags, address.get(), address.size()));
		return amount;
	}

}

#endif // __Socket_h__
