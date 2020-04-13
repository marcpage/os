#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "Socket.h"
#include "SocketGeneric.h"

/**
        @todo Test!
*/
namespace net {

/** A socket that listens for incoming connections. */
class SocketServer : public SocketGeneric {
public:
  /// @brief New Socket
  explicit SocketServer(int domain, int type = SOCK_STREAM, int protocol = 0);
  /// @brief super class behavior
  virtual ~SocketServer();
  /// @brief Bind to an address to listen to
  void bind(Address &address);
  /// @todo Document
  void reuseAddress(bool resuse = true);
  /// @todo Document
  void reusePort(bool resuse = true);
  /// @brief listen for a connection
  void listen(int backlog);
  /// @brief Wait for a connection
  void accept(Address &address, Socket &remote);
};

/**
        @param domain	The family or domain. You can use
   <code>Address.family()</code>
        @param type		The type of socket (ie SOCK_STREAM)
        @param protocol	The protocol (usually 0?)
*/
inline SocketServer::SocketServer(int domain, int type, int protocol)
    : SocketGeneric(domain, type, protocol) {}
inline SocketServer::~SocketServer() {}
/**
        @param address	The address to listen for connections
        @todo Test bind fails
*/
inline void SocketServer::bind(Address &address) {
  ErrnoOnNegative(::bind(_socket, address, address.size()));
}
/**
        @todo Document
*/
inline void SocketServer::reuseAddress(bool reuse) {
  int enable = reuse ? 1 : 0;

  ErrnoOnNegative(
      ::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)));
}
/**
        @todo Document
*/
inline void SocketServer::reusePort(bool reuse) {
  int enable = reuse ? 1 : 0;

  ErrnoOnNegative(
      ::setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)));
}
/**
        @param backlog	The number of connects that can be waiting between calls
   to <code>accept</code>.
        @todo Test listen fails
*/
inline void SocketServer::listen(int backlog) {
  ErrnoOnNegative(::listen(_socket, backlog));
}
/**
        @param address	Receives the address of the remote connection.
        @param remote	Receives the connection socket to the remote connection.
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
