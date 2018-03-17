#include "os/Thread.h"
#include "os/Mutex.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

#ifdef __Tracer_h__
	#define TestIterations	100
#else
	#define TestIterations	5000
#endif

class Adder : public exec::Thread {
	public:
		Adder(int direction, int &value, exec::Mutex &lock)
				:Thread(KeepAroundAfterFinish), _direction(direction), _lock(lock), _value(&value) {
			start();
		}
		virtual ~Adder() {}
	protected:
		virtual void *run() {
			for(int set= 0; set < TestIterations; ++set) {
				mutex_section(_lock);
				const int	initial= *_value;

				fprintf(stdout, "Beginning at %d (%d)\n", initial, _direction);
				for(int repetition= 0; repetition < 100; ++repetition) {
					dotest(*_value == initial + repetition * _direction);
					*_value+= _direction;
					fprintf(stdout, "\t Now at %d (%d)\n", *_value, _direction);
				}
				fprintf(stdout, "\t Done at %d (%d)\n", *_value, _direction);
				sleep(0.00001 /* 10 microseconds */);
			}
			return NULL;
		}
	private:
		int				_direction;
		exec::Mutex		&_lock;
		volatile int	*_value;
		Adder(const Adder&); ///< Prevent Usage
		Adder &operator=(const Adder&); ///< Prevent Usage
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	int			value;
	exec::Mutex	lock;
	Adder		onedown(-1,value,lock), oneup(1,value,lock), tendown(-10,value,lock), tenup(10,value,lock);

	onedown.join();
	oneup.join();
	tendown.join();
	tenup.join();
	return 0;
}
