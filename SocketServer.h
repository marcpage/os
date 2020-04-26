#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "Socket.h"
#include "SocketGeneric.h"

namespace net {

/// A socket that listens for incoming connections.
class SocketServer : public SocketGeneric {
public:
  /** A new socket.
        @param domain	The family or domain. You can use
                        <code>Address.family()</code>
        @param type		The type of socket (ie SOCK_STREAM)
        @param protocol	The protocol (usually 0?)
  */
  explicit SocketServer(int domain, int type = SOCK_STREAM, int protocol = 0);
  /// Destructor
  virtual ~SocketServer();
  /** Bind to an address to listen to
        @param address	The address to listen for connections
  */
  void bind(Address &address);
  /** If true, indicates that the rules used in validating addresses supplied
     should allow reuse of local addresses.
        @param reuse defaults to true
  */
  void reuseAddress(bool reuse = true);
  /** If true, permits multiple AF_INET or AF_INET6 sockets to be bound to an
              identical socket address
        @param reuse defaults to true.
        */
  void reusePort(bool reuse = true);
  /** listen for a connection
        @param backlog	The number of connects that can be waiting between calls
   to <code>accept</code>.
  */
  void listen(int backlog);
  /** Wait for a connection
        @param address	Receives the address of the remote connection.
        @param remote	Receives the connection socket to the remote connection.
  */
  void accept(Address &address, Socket &remote);
};

inline SocketServer::SocketServer(int domain, int type, int protocol)
    : SocketGeneric(domain, type, protocol) {}
inline SocketServer::~SocketServer() {}
/**
        @todo Test bind fails
*/
inline void SocketServer::bind(Address &address) {
  ErrnoOnNegative(::bind(_socket, address, address.size()));
}
inline void SocketServer::reuseAddress(bool reuse) {
  int enable = reuse ? 1 : 0;

  ErrnoOnNegative(
      ::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)));
}
inline void SocketServer::reusePort(bool reuse) {
  int enable = reuse ? 1 : 0;

  ErrnoOnNegative(
      ::setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)));
}
/**
        @todo Test listen fails
*/
inline void SocketServer::listen(int backlog) {
  ErrnoOnNegative(::listen(_socket, backlog));
}
/**
        @todo Test accept fails
*/
inline void SocketServer::accept(Address &address, Socket &remote) {
  socklen_t size = address.size();
  int socketDescriptor;

  ErrnoOnNegative(socketDescriptor = ::accept(_socket, address.get(), &size));
  remote.assign(socketDescriptor);
}
} // namespace net

#endif // __SocketServer_h__
