#ifndef __SocketGeneric_h__
#define __SocketGeneric_h__

#include "Address.h"
#include "POSIXErrno.h"
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace net {

/** Abstraction of a socket.
        @todo Add sockopt type stuff
        @todo Be able to turn signals into exceptions
*/
class SocketGeneric {
public:
  /** The socket descriptor.
          @return The socket descriptor returned by a call to
     <code>socket</code>.
  */
  int descriptor();
  /// @brief Closes the socket.
  void close();
  /// @brief Sets an option
  void setOption(int name, bool value = true, int level = SOL_SOCKET);
  /** enables permission to transmit broadcast messages */
  void broadcast(bool cast = true);
  /// @brief Bind to an address to listen to
  void bind(Address &address);

protected:
  /// @brief The socket descriptor.
  int _socket;
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

/// @todo Test
inline int SocketGeneric::descriptor() { return _socket; }
/** After closing the socket, it is made invalid.
 */
inline void SocketGeneric::close() {
  if (-1 != _socket) {
    ErrnoOnNegative(::close(_socket));
  }
  _socket = -1;
}
/// @todo Test
inline void SocketGeneric::setOption(int name, bool value, int level) {
  int enable = value ? 1 : 0;

  ErrnoOnNegative(::setsockopt(_socket, level, name, &enable, sizeof(int)));
}
inline SocketGeneric::SocketGeneric() : _socket(-1) {}
/**
        @param domain	The family or domain. You can use
                        <code>Address.family()</code>
        @param type		The type of socket (ie SOCK_STREAM)
        @param protocol	The protocol (usually 0?)
*/
inline SocketGeneric::SocketGeneric(int domain, int type, int protocol)
    : _socket(::socket(domain, type, protocol)) {
  ErrnoOnNegative(_socket);
}
/** If the socket is valid (not -1) the socket is closed.
 */
inline SocketGeneric::~SocketGeneric() {
  if (-1 != _socket) {
    try {
      close();
    } catch (const std::exception &) {
    }
  }
}
/** @param socketDescriptor	The new socket descriptor for this Socket to
 * use. */
inline void SocketGeneric::assign(int socketDescriptor) {
  _socket = socketDescriptor;
}
/**  When enabled, datagram sockets
              are allowed to send packets to a broadcast address.  This
              option has no effect on stream-oriented sockets.
              @param cast if true, the socket is set for broadcast.
              @todo Test
*/
inline void SocketGeneric::broadcast(bool cast) {
  setOption(SO_BROADCAST, cast);
}
/**
        @param address	The address to listen for connections
        @todo Test bind fails
        @todo Test
*/
inline void SocketGeneric::bind(Address &address) {
  ErrnoOnNegative(::bind(_socket, address, address.size()));
}

} // namespace net

#endif // __SocketGeneric_h__
