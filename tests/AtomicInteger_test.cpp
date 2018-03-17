#include "os/Thread.h"
#include "os/AtomicInteger.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

#ifdef __Tracer_h__
	#define TestIterations	100
#else
	#define TestIterations	800
#endif
class Adder : public exec::Thread {
	public:
		Adder(int direction, exec::AtomicInteger &value)
				:Thread(KeepAroundAfterFinish), _direction(direction), _value(value) {
			start();
		}
		virtual ~Adder() {}
	protected:
		virtual void *run() {
			for(int set= 0; set < 50; ++set) {

				fprintf(stdout, "Beginning at %d (%d)\n", _value.value(), _direction);
				for(int repetition= 0; repetition < TestIterations; ++repetition) {
					fprintf(stdout, "\t Now at %d (%d)\n", _value.valueAfterIncrement(_direction), _direction);
				}
				fprintf(stdout, "\t Done at %d (%d)\n", _value.value(), _direction);
				sleep(0.00001 /* 10 microseconds */);
			}
			return NULL;
		}
	private:
		int					_direction;
		exec::AtomicInteger	&_value;
		Adder(const Adder&); ///< Prevent Usage
		Adder &operator=(const Adder&); ///< Prevent Usage
};

/** Have four threads test atomic integer.
	Theory: If we have 2 threads modify an atomic integer by the same, but opposite, magnitudes
		we should end up with 0 at the end if the atomic integer works as expected.
		If the integer value is read from two threads and then they both write their modified value (failure)
		then we will not end up at zero.
		We use 4 threads (+1/-1 and +10/-10) and run them for 50 sets of 100 repetitions (5,000 runs)
			in hopes of catching two threads reading and writing at the same time, but not interlocked.
*/
int main(const int /*argc*/, const char * const /*argv*/[]) {
	exec::AtomicInteger	value(0);
	Adder				onedown(-1,value), oneup(1,value), tendown(-10,value), tenup(10,value);

	onedown.join();
	oneup.join();
	tendown.join();
	tenup.join();
	dotest(value == 0);
	return 0;
}
