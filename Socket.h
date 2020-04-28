#ifndef __Socket_h__
#define __Socket_h__

#include "SocketGeneric.h"

namespace net {

/// A standard socket that you expect to read/write.
class Socket : public SocketGeneric {
public:
  /// Should routing be used?
  enum RoutingOptions {
    Route,
    /** Don't use a gateway to send out the packet, only send to hosts on
     * directly connected networks. This is usually used only by diagnostic or
     * routing programs. This is only defined for protocol families that route;
     * packet sockets don't.
     */
    BypassRouting
  };
  /// How should the data be sent?
  enum OutOfBandDataOptions {
    IgnoreOutOfBand,
    /** The stream socket abstraction includes out-of-band data. Out-of-band
     * data is a logically independent transmission channel between a pair of
     * connected stream sockets. Out-of-band data is delivered independent of
     * normal data. The out-of-band data facilities must support the reliable
     * delivery of at least one out-of-band message at a time. This message can
     * contain at least one byte of data. At least one message can be pending
     * delivery at any time.
     */
    ProcessOutOfBand
  };
  /// Invalid socket.
  Socket();
  /** Creates a new socket.
        @param domain	The domain or family <code>Address.family()</code>
        @param type		The type of socket (ie SOCK_STREAM)
        @param protocol	The socket protocol (usually 0?)
  */
  explicit Socket(int domain, int type = SOCK_STREAM, int protocol = 0);
  /// Destructor.
  virtual ~Socket();
  /** Connect to a given address.
        @param address	The address to connect to.
  */
  void connect(Address &address);
  /** Read bytes into a buffer from the socket.
        @param bytes	The number of bytes to but in the buffer, or -1 for
   buffer max. If <code>bytes</code> is greater than the buffer size, the buffer
   max will be used.
  */
  std::string read(size_t bytes);
  /** Read bytes into a buffer from the socket.
        @param bytes	The number of bytes to but in the buffer, or -1 for
                        buffer max. If <code>bytes</code> is greater than the
                        buffer size, the buffer max will be used.
        @param buffer	The buffer to store the bytes in, also returned for
                                                convenience
  */
  std::string &read(size_t bytes, std::string &buffer);
  /** Write bytes from a buffer to the socket.
        @param buffer	The buffer to send
        @param bytes	The number of bytes in the buffer to send, or -1 for the
                        entire buffer max. If <code>bytes</code> is greater than
                        the buffer size, the buffer max will be used.
        @param offset	The offset in the buffer to start writing bytes from.
  */
  size_t write(const std::string &buffer,
               size_t bytes = static_cast<size_t>(-1), size_t offset = 0);
  /** Send data to an address without connecting.
        @param address The address to send the data to.
        @param buffer The data to send.
        @param bytes The number of bytes to send. Defaults to rest of the buffer
        @param offset The offset in buffer to start sending bytes. Defaults to 0
        @param route How should the data be routed. Defaults to Route
        @param outOfBand How should the data be sent. Defaults to
     IgnoreOutOfBand
  */
  size_t sendTo(const Address &address, const std::string &buffer,
                size_t bytes = static_cast<size_t>(-1), size_t offset = 0,
                RoutingOptions route = Route,
                OutOfBandDataOptions outOfBand = IgnoreOutOfBand);
};

inline Socket::Socket() : SocketGeneric() {}
inline Socket::Socket(int domain, int type, int protocol)
    : SocketGeneric(domain, type, protocol) {}
inline Socket::~Socket() {}
inline void Socket::connect(Address &address) {
  ErrnoOnNegative(::connect(_socket, address, address.size()));
}
inline std::string Socket::read(size_t bytes) {
  std::string buffer;
  return read(bytes, buffer);
}
inline std::string &Socket::read(size_t bytes, std::string &buffer) {
  ssize_t amount;

  buffer.assign(bytes, '\0');
  ErrnoOnNegative(
      amount = ::read(_socket, const_cast<char *>(buffer.data()), bytes));
  buffer.erase(amount);
  return buffer;
}
inline size_t Socket::write(const std::string &buffer, size_t bytes,
                            size_t offset) {
  ssize_t amount;
  size_t toWrite =
      bytes == static_cast<size_t>(-1) ? buffer.size() - offset : bytes;

  if (offset + toWrite > buffer.size()) {
    toWrite = buffer.size() - offset;
  }
  ErrnoOnNegative(amount = ::write(_socket, buffer.data() + offset, toWrite));
  return amount;
}
/// @todo Test
inline size_t Socket::sendTo(const Address &address, const std::string &buffer,
                             size_t bytes, size_t offset, RoutingOptions route,
                             OutOfBandDataOptions outOfBand) {
  ssize_t amount;
  const size_t toSend =
      bytes == static_cast<size_t>(-1) ? buffer.size() - offset : bytes;
  const int flags = (BypassRouting == route ? MSG_DONTROUTE : 0) |
                    (IgnoreOutOfBand == outOfBand ? MSG_OOB : 0);

  ErrnoOnNegative(amount = ::sendto(_socket, buffer.data() + offset, toSend,
                                    flags, address.get(), address.size()));
  return amount;
}

} // namespace net

#endif // __Socket_h__
