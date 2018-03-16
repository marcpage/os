#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "Address.h"
#include "POSIXErrno.h"

namespace net {

	/** Abstraction of a socket.
		@todo Add sockopt type stuff
		@todo Be able to turn signals into exceptions
	*/
	class SocketGeneric {
		public:
			/// @brief The socket descriptor.
			int descriptor();
			/// @brief Closes the socket.
			void close();
		protected:
			/// @brief The socket descriptor.
			int	_socket;
			/// @brief Invalid socket.
			SocketGeneric();
			/// @brief subclass constructor
			SocketGeneric(int domain, int type, int protocol);
			/// @brief closes the socket if it's valid.
			virtual ~SocketGeneric();
			/// @brief set the descriptor
			void assign(int socketDescriptor);
			friend class SocketServer; ///< For <code>assign()</code>
	};

	/** @return The socket descriptor returned by a call to <code>socket</code>. */
	inline int SocketGeneric::descriptor() {
		return _socket;
	}
	/** After closing the socket, it is made invalid.
	*/
	inline void SocketGeneric::close() {
		if (-1 != _socket) {
			ErrnoOnNegative(::close(_socket));
		}
		_socket= -1;
	}
	inline SocketGeneric::SocketGeneric()
		:_socket(-1) {
	}
	/**
		@param domain	The family or domain. You can use <code>Address.family()</code>
		@param type		The type of socket (ie SOCK_STREAM)
		@param protocol	The protocol (usually 0?)
	*/
	inline SocketGeneric::SocketGeneric(int domain, int type, int protocol)
		:_socket(::socket(domain, type, protocol)) {
		ErrnoOnNegative(_socket);
	}
	/** If the socket is valid (not -1) the socket is closed.
	*/
	inline SocketGeneric::~SocketGeneric() {
		if(-1 != _socket) {
			try	{
				close();
			} catch(const std::exception&) {
			}
		}
	}
	/** @param socketDescriptor	The new socket descriptor for this Socket to use. */
	inline void SocketGeneric::assign(int socketDescriptor) {
		_socket= socketDescriptor;
	}

}

#endif // __SocketGeneric_h__
