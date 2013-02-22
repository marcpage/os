#include "os/SocketServer.h"
#include "os/AddressIPv4.h"
#include "os/AddressIPv6.h"
#include "os/BufferManaged.h"
#include "os/BufferString.h"
#include "os/Thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

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
					printf("received->sent('%s')\n", std::string(reinterpret_cast<char*>(buffer.start()), amountIn).c_str());
					amountOut= _connection->write(buffer, amountIn);
					if(amountOut != amountIn) {
						printf("FAILED: we read %ld but we wrote %ld\n", amountIn, amountOut);
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
};

class Server : public exec::Thread {
	public:
		Server(int port)
				:exec::Thread(KeepAroundAfterFinish),
				_serverAddress(port),
				_server(_serverAddress.family()),
				_exiting(false),
				_threads() {
			_server.bind(_serverAddress);
			_server.listen(1);
			start();
		}
		virtual ~Server() {
			for(ServerThreads::iterator thread= _threads.begin(); thread != _threads.end(); ++thread) {
				if((*thread)->running()) {
					printf("FAIL: thread failed to exit\n");
				}
				delete *thread;
			}
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

					printf("Waiting for connection\n");
					_server.accept(connectedTo, *connection);
					printf("Connection received\n");
					_threads.push_back(new Echo(connection));
				}
			} catch(const std::exception &exception) {
				if(_exiting) {
					printf("Server Thread: %s\n", exception.what());
				} else {
					printf("FAILED: Server Thread Exception: %s\n", exception.what());
				}
			}
			for(ServerThreads::iterator thread= _threads.begin(); thread != _threads.end(); ++thread) {
				if((*thread)->running()) {
					(*thread)->shutdown();
					(*thread)->join();
				}
			}
			return NULL;
		}
		virtual void *handle(const std::exception &exception, void *result) {
			printf("FAIL: Exception: %s\n", exception.what());
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

int main(const int argc, const char * const argv[]) {
	try	{
		in_port_t			port= (argc == 2) ? atoi(argv[1]) : 8086;
		Server				server(port);
		net::AddressIPv4	local(port);
		net::Socket			connection(local.family());
		BufferManaged		readBuffer(4096);
		std::string			writeString;
		BufferString		writeBuffer(writeString);
		size_t				amount;

		printf("Connecting\n");
		connection.connect(local);
		writeString= "Hello";
		printf("Writing %s\n", writeString.c_str());
		amount= connection.write(writeBuffer);
		if(amount != writeString.size()) {
			printf("FAIL: Unable to write Hello\n");
		}
		printf("Reading\n");
		amount= connection.read(readBuffer, amount);
		if(amount != writeString.size()) {
			printf("FAIL: Unable to read Hello\n");
		}
		printf("Closing\n");
		connection.close();
		server.shutdown();
	} catch(const std::exception &exception) {
		printf("FAILED: exception thrown on main thread: %s\n", exception.what());
	}
	return 0;
}
