#include "os/Signal.h"
#include "os/Thread.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

#ifdef __Tracer_h__
	#define TestIterations	10
#else
	#define TestIterations	50
#endif
class Adder : public exec::Thread {
	public:
		Adder(int direction, int &value, exec::Signal &signal, exec::Mutex &lock)
				:Thread(KeepAroundAfterFinish), _direction(direction), _running(true), _value(value), _lock(lock), _signal(signal) {
			start();
		}
		void stop() {_running= false;}
		virtual ~Adder() {}
	protected:
		virtual void *run() {
			while(_running) {
				exec::Mutex::Locker	lock(_lock);

				_signal.wait(_lock);
				if(_running) {
					_value+= _direction;
				}
			}
			return NULL;
		}
	private:
		int				_direction;
		bool			_running;
		int				&_value;
		exec::Mutex		&_lock;
		exec::Signal	&_signal;
		Adder(const Adder&); ///< Prevent Usage
		Adder &operator=(const Adder&); ///< Prevent Usage
};

/** Have 6 threads
	Theory: If we have 2 threads modify an integer by the same, but opposite, magnitudes
		we should end up with 0 at the end if the atomic integer works as expected.
		If the integer value is read from two threads and then they both write their modified value (failure)
		then we will not end up at zero.
		We use 6 threads (+1/-1, +10/-10, +100/-100). When we signal we should get either +/-1, +/-10 or +/-100.
		When we broadcast, we should get 0, as all threads balance out.
		We don't need the integer we increment/decrement to be atomic because it is protected by the mutex.
*/
int main(const int /*argc*/, const char * const /*argv*/[]) {
	int				value;
	exec::Signal	signal;
	exec::Mutex		lock;
	Adder			onedown(-1,value, signal, lock), oneup(1,value, signal, lock),
					tendown(-10,value, signal, lock), tenup(10,value, signal, lock),
					hundreddown(-100,value, signal, lock), hundredup(100,value, signal, lock);

	for(int iterations= 0; iterations < TestIterations; ++iterations) {
		value= 0;
		exec::Thread::sleep(5, exec::Thread::Milliseconds);
		dotest(value == 0);
		signal.signal();
		exec::Thread::sleep(5, exec::Thread::Milliseconds);
		dotest(value != 0);
		printf("The thread that won: %d\n", value);
	}
	for(int iterations= 0; iterations < TestIterations; ++iterations) {
		value= 0;
		exec::Thread::sleep(5, exec::Thread::Milliseconds);
		dotest(value == 0);
		signal.broadcast();
		exec::Thread::sleep(5, exec::Thread::Milliseconds);
		dotest(value == 0);
	}
	onedown.stop();
	oneup.stop();
	tendown.stop();
	tenup.stop();
	hundreddown.stop();
	hundredup.stop();
	signal.broadcast(); // let all the threads recognize that they are not running anymore
	onedown.join();
	oneup.join();
	tendown.join();
	tenup.join();
	hundreddown.join();
	hundredup.join();
	return 0;
}
