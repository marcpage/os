#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include "os/SocketServer.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"
#include "os/BufferManaged.h"
#include "os/BufferString.h"

void Echo(net::Socket *connection) {
	BufferManaged	buffer(4096);
	size_t			amountOut, amountIn= connection->read(buffer);

	while(amountIn > 0) {
		printf("THREAD: received->sent('%s')\n",
				std::string(reinterpret_cast<char*>(buffer.start()), amountIn).c_str());
		amountOut= connection->write(buffer, amountIn);
		if(amountOut != amountIn) {
			printf("THREAD: FAILED: we read %ld but we wrote %ld\n", amountIn, amountOut);
		}
		amountIn= connection->read(buffer);
	}
	connection->close();
	delete connection;
}

bool running = true;

void Server(net::SocketServer *server, net::AddressIPv4 *serverAddress) {
	std::vector<std::thread>	threads;

	server->reuseAddress();
	server->reusePort();
	server->bind(*serverAddress);
	server->listen(1);

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
	int	iterations= 5300;
#ifdef __Tracer_h__
	iterations= 3;
#endif
	try	{
		in_port_t			port= (argc == 2) ? atoi(argv[1]) : 8081;
		net::AddressIPv4	serverAddress(port);
		net::SocketServer	serverSocket(serverAddress.family());
		std::thread			server(Server, &serverSocket, &serverAddress);
		BufferManaged		readBuffer(4096);
		std::string			writeString;
		BufferString		writeBuffer(writeString);
		size_t				amount;

		std::this_thread::sleep_for(std::chrono::milliseconds(1)); // wait for server thread to come up
		for(int i= 0; i < iterations; ++i) {
			try {
				net::AddressIPv4	local(port);
				net::Socket			connection(local.family());

				printf("THREAD: Connecting\n");
				connection.connect(local);
				writeString= "Hello";
				printf("THREAD: Writing %s\n", writeString.c_str());
				amount= connection.write(writeBuffer);
				if(amount != writeString.size()) {
					printf("THREAD: FAIL: Unable to write Hello\n");
				}
				printf("THREAD: Reading\n");
				amount= connection.read(readBuffer, amount);
				if(amount != writeString.size()) {
					printf("THREAD: FAIL: Unable to read Hello\n");
				}
				printf("THREAD: Closing\n");
				connection.close();
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
