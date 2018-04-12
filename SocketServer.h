#ifndef __SocketServer_h__
#define __SocketServer_h__

#include "os/SocketGeneric.h"
#include "os/Socket.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"
#include <arpa/inet.h>

/**
	@todo Test!
	@todo Test udp server and client
	@todo Test getting dns name
*/
namespace net {

	/** A socket that listens for incoming connections. */
	class SocketServer : public SocketGeneric {
		public:
			/** @todo Document */
			enum PartialBehavior {
				AcceptPartial,
				AcceptFull
			};
			/** @todo Document */
			enum MessageBehavior {
				ConsumeMessage,
				PeekAtMessage
			};
			/** @todo Document */
			enum OutOfBandDataOptions {
				IgnoreOutOfBand,
				ProcessOutOfBand
			};
			typedef Address *AddressPtr;
			/// @brief New Socket
			SocketServer(int domain, int type= SOCK_STREAM, int protocol= 0);
			/// @brief super class behavior
			virtual ~SocketServer();
			/// @todo Document
			void reuseAddress(bool resuse=true);
			/// @todo Document
			void reusePort(bool resuse=true);
			/// @brief listen for a connection
			void listen(int backlog);
			/// @brief Wait for a connection
			void accept(Address &address, Socket &remote);
			/// @todo Document
			std::string receiveFrom(size_t maximumSize, AddressPtr &sender, PartialBehavior partial=AcceptFull, MessageBehavior behavior=ConsumeMessage, OutOfBandDataOptions outOfBand=IgnoreOutOfBand);
	};

	/**
		@param domain	The family or domain. You can use <code>Address.family()</code>
		@param type		The type of socket (ie SOCK_STREAM)
		@param protocol	The protocol (usually 0?)
	*/
	inline SocketServer::SocketServer(int domain, int type, int protocol)
		:SocketGeneric(domain, type, protocol) {
	}
	inline SocketServer::~SocketServer() {}
	/**
		@todo Document
	*/
	inline void SocketServer::reuseAddress(bool reuse) {
		setOption(SO_REUSEADDR, reuse);
	}
	/**
		@todo Document
	*/
	inline void SocketServer::reusePort(bool reuse) {
		setOption(SO_REUSEPORT, reuse);
	}
	/**
		@param backlog	The number of connects that can be waiting between calls to <code>accept</code>.
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
		socklen_t	size= address.size();
		int			socketDescriptor;

		ErrnoOnNegative(socketDescriptor= ::accept(_socket, address.get(), &size));
		remote.assign(socketDescriptor);
	}
	/**
		@todo Document
	*/
	inline std::string SocketServer::receiveFrom(size_t maximumSize, AddressPtr &sender, PartialBehavior partial, MessageBehavior behavior, OutOfBandDataOptions outOfBand) {
		ssize_t 			actualSize;
		std::string 		data(maximumSize, '\0');
		std::string 		address(std::max(size_t(AddressIPv6::Size), size_t(AddressIPv4::Size)), '\0');
		socklen_t			addressSize= address.size();
		struct sockaddr		*genericAddress= reinterpret_cast<struct sockaddr*>(const_cast<char*>(address.data()));
		const int			flags= (AcceptFull == partial ? MSG_WAITALL : 0) | (PeekAtMessage == behavior ? MSG_PEEK : 0) | (ProcessOutOfBand == outOfBand ? MSG_OOB : 0);

		AssertMessageException(NULL == sender);
		ErrnoOnNegative(actualSize= ::recvfrom(_socket, const_cast<char*>(data.data()), data.size(), flags, genericAddress, &addressSize));
		AssertMessageException( (net::AddressIPv4::Family == genericAddress->sa_family) || (net::AddressIPv6::Family == genericAddress->sa_family) );
		printf("genericAddress->sa_family=%d net::AddressIPv4::Family=%d\n", genericAddress->sa_family, net::AddressIPv4::Family);
		if (net::AddressIPv4::Family == genericAddress->sa_family) {
			sender= new net::AddressIPv4(genericAddress, addressSize);
		} else {
			sender= new net::AddressIPv6(genericAddress, addressSize);
		}
		data.erase(actualSize);
		return data;
	}
}

#endif // __SocketServer_h__
