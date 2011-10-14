#include "os/SocketServer.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"
#include "os/BufferManaged.h"
#include <stdio.h>
#include <stdlib.h>

int main(const int argc, const char * const argv[]) {
	in_port_t			port= (argc == 2) ? atoi(argv[1]) : 8086;
	net::AddressIPv4	serverAddress(port);
	net::SocketServer	server(serverAddress.family());
	net::Socket			connection;
	net::AddressIPv6	connectedTo;
	BufferManaged		buffer(4096);
	size_t				amount;

	server.bind(serverAddress);
	server.listen(1);
	while(true) {
		server.accept(connectedTo, connection);
		while(true) {
			amount= connection.read(buffer);
			connection.write(buffer, amount);
		}
	}
	return 0;
}
