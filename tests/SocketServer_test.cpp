#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "os/Thread.h"
#include "os/SocketServer.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"
#include "os/BufferManaged.h"
#include "os/BufferString.h"

class Echo : public exec::Thread {
	public:
		Echo(net::Socket *connection)
				:exec::Thread(KeepAroundAfterFinish),
				_connection(connection),
				_running(true) {
			start();
		}
		virtual ~Echo() {
			delete _connection;
		}
		bool running() {return _running;}
		void shutdown() {
			_running= false;
			_connection->close();
		}
	protected:
		virtual void *run() {
			size_t			amountOut, amountIn= 1;
			BufferManaged	buffer(4096);

			while( _running && (amountIn > 0) ) {
				amountIn= _connection->read(buffer);
				if(0 == amountIn) {
					shutdown();
				} else {
					printf("THREAD: %p: received->sent('%s')\n", exec::ThreadId::current().thread(), std::string(reinterpret_cast<char*>(buffer.start()), amountIn).c_str());
					amountOut= _connection->write(buffer, amountIn);
					if(amountOut != amountIn) {
						printf("THREAD: %p: FAILED: we read %ld but we wrote %ld\n", exec::ThreadId::current().thread(), amountIn, amountOut);
					}
				}
			}
			_running= false;
			_connection->close();
			return NULL;
		}
	private:
		net::Socket	*_connection;
		bool		_running;
		Echo(const Echo&); ///< Prevent Usage
		Echo &operator=(const Echo&); ///< Prevent Usage
};

class Server : public exec::Thread {
	public:
		Server(int port)
				:exec::Thread(KeepAroundAfterFinish),
				_serverAddress(port),
				_server(_serverAddress.family()),
				_exiting(false),
				_threads() {
			_server.reuseAddress();
			_server.reusePort();
			_server.bind(_serverAddress);
			_server.listen(1);
			start();
		}
		virtual ~Server() {
			for(ServerThreads::iterator thrd= _threads.begin(); thrd != _threads.end(); ++thrd) {
				if((*thrd)->running()) {
					printf("THREAD: %p: FAIL: thread failed to exit\n", exec::ThreadId::current().thread());
				}
				delete *thrd;
				*thrd= NULL;
			}
			_threads.clear();
		}
		void shutdown() {
			_exiting= true;
			_server.close();
			join();
		}
	protected:
		virtual void *run() {
			try	{
				while(!_exiting) {
					net::AddressIPv6	connectedTo;
					net::Socket			*connection= new net::Socket();

					printf("THREAD: %p: Waiting for connection\n", exec::ThreadId::current().thread());
					_server.accept(connectedTo, *connection);
					printf("THREAD: %p: Connection received\n", exec::ThreadId::current().thread());
					_threads.push_back(new Echo(connection));
					while (_threads.size() > 1000) {
						for (ServerThreads::iterator thread = _threads.begin(); thread != _threads.end(); ++thread) {
							if (!(*thread)->running()) {
								delete *thread;
								_threads.erase(thread);
								break;
							}
						}
					}
				}
			} catch(const std::exception &exception) {
				if(_exiting) {
					printf("THREAD: %p: EXPECTED: Server Thread: %s\n", exec::ThreadId::current().thread(), exception.what());
				} else {
					printf("THREAD: %p: FAILED: Server Thread Exception: %s\n", exec::ThreadId::current().thread(), exception.what());
				}
			}
			for(ServerThreads::iterator thrd= _threads.begin(); thrd != _threads.end(); ++thrd) {
				if((*thrd)->running()) {
					(*thrd)->shutdown();
					(*thrd)->join();
				}
			}
			return NULL;
		}
		virtual void *handle(const std::exception &exception, void *result) {
			printf("THREAD: %p: FAIL: Exception: %s\n", exec::ThreadId::current().thread(), exception.what());
			return result;
		}
	private:
		typedef std::vector<Echo*>	ServerThreads;
		net::AddressIPv4	_serverAddress;
		net::SocketServer	_server;
		bool				_exiting;
		ServerThreads		_threads;
		Server(const Server&); ///< Prevent Usage
		Server &operator=(const Server&); ///< Prevent Usage
};

class UDPServer : public exec::Thread {
	public:
		UDPServer(in_port_t serverPort)
				:exec::Thread(KeepAroundAfterFinish),
				_serverAddress(serverPort, INADDR_ANY),
				_server(_serverAddress.family(), SOCK_DGRAM) {
			_server.broadcast();
			_server.bind(_serverAddress);
		}
		virtual ~UDPServer() {}
		void close() {_server.close();}
	protected:
		virtual void *run() {
			try {
				while(true) {
					net::SocketServer::AddressPtr	from= NULL;
					std::string 					input= _server.receiveFrom(4096, from);

					if (input.length() == 0) {
						break;
					}
					printf("Received: '%s'\n", input.c_str());
				}
			} catch(const std::exception &exception) {
				printf("Exception: %s\n", exception.what());
			}
			return NULL;
		}
	private:
		net::AddressIPv4	_serverAddress;
		net::SocketServer	_server;
		UDPServer(const UDPServer&); ///< Prevent Usage
		UDPServer &operator=(const UDPServer&); ///< Prevent Usage
};

void testUDP() {
	const in_port_t		port= 1500;
	UDPServer			serverThread(port);
	net::AddressIPv4	server("localhost", port);
	net::AddressIPv4	localAddress(in_port_t(0), INADDR_ANY);
	net::Socket			connection(server.family(), SOCK_DGRAM);
	std::string			message;
	BufferString		writeBuffer(message);

	connection.broadcast();
	connection.bind(localAddress);
	message= "Testing UDP";
	connection.sendTo(server, writeBuffer);
	message= "Testing UDP Again";
	connection.sendTo(server, writeBuffer);
	serverThread.close();
}

int main(const int argc, const char * const argv[]) {
	int	iterations= 400;
#ifdef __Tracer_h__
	iterations= 3;
#endif
	try	{
		in_port_t			port= (argc == 2) ? atoi(argv[1]) : 8081;
		Server				server(port);
		BufferManaged		readBuffer(4096);
		std::string			writeString;
		BufferString		writeBuffer(writeString);
		size_t				amount;

		for(int i= 0; i < iterations; ++i) {
			try {
				net::AddressIPv4	local(port);
				net::Socket			connection(local.family());

				printf("THREAD: %p: Connecting\n", exec::ThreadId::current().thread());
				connection.connect(local);
				writeString= "Hello";
				printf("THREAD: %p: Writing %s\n", exec::ThreadId::current().thread(), writeString.c_str());
				amount= connection.write(writeBuffer);
				if(amount != writeString.size()) {
					printf("THREAD: %p: FAIL: Unable to write Hello\n", exec::ThreadId::current().thread());
				}
				printf("THREAD: %p: Reading\n", exec::ThreadId::current().thread());
				amount= connection.read(readBuffer, amount);
				if(amount != writeString.size()) {
					printf("THREAD: %p: FAIL: Unable to read Hello\n", exec::ThreadId::current().thread());
				}
				printf("THREAD: %p: Closing\n", exec::ThreadId::current().thread());
				connection.close();

				try {
					printf("A %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::FullyQualified, net::Address::Numeric, net::Address::NameRequired).c_str());
					printf("FAILED: (A) We should have gotten EAI_NONAME\n");
				} catch(const net::EAI_NONAME_GAI &) {
				}
				try {
					printf("B %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::UnqualifiedLocal, net::Address::Numeric, net::Address::NameRequired).c_str());
					printf("FAILED: (B) We should have gotten EAI_NONAME\n");
				} catch(const net::EAI_NONAME_GAI &) {
				}
				printf("C %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::FullyQualified, net::Address::Name, net::Address::NameRequired).c_str());
				printf("D %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::UnqualifiedLocal, net::Address::Name, net::Address::NameRequired).c_str());
				printf("E %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::FullyQualified, net::Address::Numeric, net::Address::NameIfAvailable).c_str());
				printf("F %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::UnqualifiedLocal, net::Address::Numeric, net::Address::NameIfAvailable).c_str());
				printf("G %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::FullyQualified, net::Address::Name, net::Address::NameIfAvailable).c_str());
				printf("H %s\n", net::AddressIPv4("127.0.0.1", 80).name(net::Address::UnqualifiedLocal, net::Address::Name, net::Address::NameIfAvailable).c_str());

				try {
					printf("I %s\n", net::AddressIPv4("localhost", 80).name(net::Address::FullyQualified, net::Address::Numeric, net::Address::NameRequired).c_str());
					printf("FAILED: (I) We should have gotten EAI_NONAME\n");
				} catch(const net::EAI_NONAME_GAI &) {
				}
				try {
					printf("J %s\n", net::AddressIPv4("localhost", 80).name(net::Address::UnqualifiedLocal, net::Address::Numeric, net::Address::NameRequired).c_str());
					printf("FAILED: (J) We should have gotten EAI_NONAME\n");
				} catch(const net::EAI_NONAME_GAI &) {
				}
				printf("K %s\n", net::AddressIPv4("localhost", 80).name(net::Address::FullyQualified, net::Address::Name, net::Address::NameRequired).c_str());
				printf("L %s\n", net::AddressIPv4("localhost", 80).name(net::Address::UnqualifiedLocal, net::Address::Name, net::Address::NameRequired).c_str());
				printf("M %s\n", net::AddressIPv4("localhost", 80).name(net::Address::FullyQualified, net::Address::Numeric, net::Address::NameIfAvailable).c_str());
				printf("N %s\n", net::AddressIPv4("localhost", 80).name(net::Address::UnqualifiedLocal, net::Address::Numeric, net::Address::NameIfAvailable).c_str());
				printf("O %s\n", net::AddressIPv4("localhost", 80).name(net::Address::FullyQualified, net::Address::Name, net::Address::NameIfAvailable).c_str());
				printf("P %s\n", net::AddressIPv4("localhost", 80).name(net::Address::UnqualifiedLocal, net::Address::Name, net::Address::NameIfAvailable).c_str());

				try {
					GAIMessageThrow(EAI_AGAIN);
					printf("FAILED: Should have thrown EAI_AGAIN\n");
				} catch(const net::EAI_AGAIN_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_FAIL);
					printf("FAILED: Should have thrown EAI_FAIL\n");
				} catch(const net::EAI_FAIL_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_SERVICE);
					printf("FAILED: Should have thrown EAI_SERVICE\n");
				} catch(const net::EAI_SERVICE_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_BADFLAGS);
					printf("FAILED: Should have thrown EAI_BADFLAGS\n");
				} catch(const net::EAI_BADFLAGS_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_FAMILY);
					printf("FAILED: Should have thrown EAI_FAMILY\n");
				} catch(const net::EAI_FAMILY_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_SOCKTYPE);
					printf("FAILED: Should have thrown EAI_SOCKTYPE\n");
				} catch(const net::EAI_SOCKTYPE_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_BADHINTS);
					printf("FAILED: Should have thrown EAI_BADHINTS\n");
				} catch(const net::EAI_BADHINTS_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_MEMORY);
					printf("FAILED: Should have thrown EAI_MEMORY\n");
				} catch(const net::EAI_MEMORY_GAI&) {
				}
				try {
					GAIMessageThrow(EAI_SYSTEM);
					printf("FAILED: Should have thrown EAI_SYSTEM\n");
				} catch(const net::EAI_SYSTEM_GAI&) {
				}
				try {
					GAIMessageThrow(8086);
					printf("FAILED: Should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_NONAME_GAI &) {
					printf("FAILED: EAI_NONAME_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_AGAIN_GAI&) {
					printf("FAILED: EAI_AGAIN_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_FAIL_GAI&) {
					printf("FAILED: EAI_FAIL_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_SERVICE_GAI&) {
					printf("FAILED: EAI_SERVICE_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_BADFLAGS_GAI&) {
					printf("FAILED: EAI_BADFLAGS_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_FAMILY_GAI&) {
					printf("FAILED: EAI_FAMILY_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_SOCKTYPE_GAI&) {
					printf("FAILED: EAI_SOCKTYPE_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_BADHINTS_GAI&) {
					printf("FAILED: EAI_BADHINTS_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_MEMORY_GAI&) {
					printf("FAILED: EAI_MEMORY_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::EAI_SYSTEM_GAI&) {
					printf("FAILED: EAI_SYSTEM_GAI, but should have thrown GetAddressInfoException\n");
				} catch(const net::GetAddressInfoException &exception) {
					printf("We got unknown exception %s\n", exception.name());
				}

				testUDP();
			} catch(const std::exception &exception) {
				printf("THREAD: %p: FAILED: exception thrown on main thread, iteration %d: %s\n", exec::ThreadId::current().thread(), i, exception.what());
			}
		}
		server.shutdown();
	} catch(const std::exception &exception) {
		printf("THREAD: %p: FAILED: exception thrown on main thread: %s\n", exec::ThreadId::current().thread(), exception.what());
	}
	return 0;
}
