#include "Thread.h"
#include "AtomicInteger.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

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
				for(int repetition= 0; repetition < 100; ++repetition) {
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
