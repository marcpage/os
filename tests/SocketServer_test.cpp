#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include "os/SocketServer.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"

void Echo(net::Socket *connection) {
	std::string		buffer = connection->read(4096);
	size_t			amountOut;

	while(buffer.size() > 0) {
		printf("THREAD: received->sent('%s')\n", buffer.c_str());
		amountOut= connection->write(buffer);
		if(amountOut != buffer.size()) {
			printf("THREAD: FAILED: we read %ld but we wrote %ld\n", buffer.size(), amountOut);
		}
		buffer= connection->read(4096);
	}
	delete connection;
}

bool running = true;

void Server(net::SocketServer *server) {
	std::vector<std::thread>	threads;

	try {
		while(running) {
			net::AddressIPv6	connectedTo;
			net::Socket			*connection= new net::Socket();

			printf("THREAD: Waiting for connection\n");
			server->accept(connectedTo, *connection);
			printf("THREAD: Connection received\n");
			threads.push_back(std::thread(Echo, connection));
		}
	} catch(const posix::err::ECONNABORTED_Errno &) {
	} catch(const std::exception &exception) {
		printf("FAIL: Unexpected exception %s\n", exception.what());
	}
	for (std::vector<std::thread>::iterator thread = threads.begin(); thread != threads.end(); ++thread) {
		thread->join();
	}
}

int main(const int argc, const char * const argv[]) {
	int	iterations= 5000; // 5800 caused EADDRNOTAVAIL on the 3rd version about the last 3,000 connects
#ifdef __Tracer_h__
	iterations= 3;
#endif
	try	{
		in_port_t			port= (argc == 2) ? atoi(argv[1]) : 8082;
		net::AddressIPv4	serverAddress(port);
		net::SocketServer	serverSocket(serverAddress.family());

		serverSocket.reuseAddress();
		serverSocket.reusePort();
		serverSocket.bind(serverAddress);
		serverSocket.listen(100);

		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		std::thread			server(Server, &serverSocket);
		std::string			readString;
		std::string			writeString;
		size_t				amount;

		for(int i= 0; i < iterations; ++i) {
			try {
				net::AddressIPv4	local(port);
				net::Socket			connection(local.family());

				printf("THREAD: Connecting\n");
				connection.connect(local);
				if (writeString.size() > 1024) {
					writeString = "";
				}
				writeString+= "he";
				printf("THREAD: Writing %s\n", writeString.c_str());
				amount= connection.write(writeString, writeString.size() + 20);
				if(amount != writeString.size()) {
					printf("THREAD: FAIL: Unable to write Hello\n");
				}
				printf("THREAD: Reading\n");
				readString= connection.read(amount);
				if(readString != writeString) {
					printf("THREAD: FAIL: Unable to read Hello\n");
				}
				printf("THREAD: Closing\n");
				connection.close();
			} catch(const posix::err::EADDRNOTAVAIL_Errno &exception) {
				printf("THREAD: FAILED: appears address is not available (connect?): %s\n", exception.what());
			} catch(const std::exception &exception) {
				printf("THREAD: FAILED: exception thrown on main thread, iteration %d: %s\n", i, exception.what());
			}
		}
		serverSocket.close();
		running = false;
		server.join();
	} catch(const std::exception &exception) {
		printf("THREAD: FAILED: exception thrown on main thread: %s\n", exception.what());
	}
	return 0;
}
