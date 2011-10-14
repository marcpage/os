#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include <sys/socket.h>
#include <string>
#include "Buffer.h"
#include "Address.h"
#include "Exception.h"

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

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
	inline int SocketGeneric::descriptor() {trace_scope
		return _socket;
	}
	/** After closing the socket, it is made invalid.
	*/
	inline void SocketGeneric::close() {trace_scope
		errnoAssertPositiveMessageException(::close(_socket));
		_socket= -1;
	}
	inline SocketGeneric::SocketGeneric()
		:_socket(-1) {trace_scope
	}
	/**
		@param domain	The family or domain. You can use <code>Address.family()</code>
		@param type		The type of socket (ie SOCK_STREAM)
		@param protocol	The protocol (usually 0?)
	*/
	inline SocketGeneric::SocketGeneric(int domain, int type, int protocol)
		:_socket(::socket(domain, type, protocol)) {trace_scope
		errnoAssertPositiveMessageException(_socket);
	}
	/** If the socket is valid (not -1) the socket is closed.
	*/
	inline SocketGeneric::~SocketGeneric() {trace_scope
		if(-1 != _socket) {
			try	{
				close();
			} catch(const std::exception&) {trace_scope
			}
		}
	}
	/** @param socketDescriptor	The new socket descriptor for this Socket to use. */
	inline void SocketGeneric::assign(int socketDescriptor) {trace_scope
		_socket= socketDescriptor;
	}

}

#endif // __SocketGeneric_h__
