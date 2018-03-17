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

int main(const int argc, const char * const argv[]) {
	int	iterations= 5000;
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
