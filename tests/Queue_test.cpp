#include "os/Signal.h"
#include "os/Thread.h"
#include "os/Queue.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

#ifdef __Tracer_h__
	#define TestIterations	1000
#else
	#define TestIterations	30000
#endif
class PassTheBuck : public exec::Thread {
	public:
		typedef exec::Queue<int>	IntQueue;
		PassTheBuck(IntQueue &in, IntQueue &out)
				:Thread(KeepAroundAfterFinish), _running(true), _in(in), _out(out) {
			start();
		}
		void done() {_running= false;}
		virtual ~PassTheBuck() {}
	protected:
		virtual void *run() {
			try	{
				while(_running || !_in.empty()) {
					_out.enqueue(_in.dequeue());
					exec::Thread::sleep(5, exec::Thread::Milliseconds);
				}
			} catch(const std::exception &exception) {
				fprintf(stderr, "EXCEPTION: %s id=%p\n", exception.what(),this);
			}
			return NULL;
		}
	private:
		bool		_running;
		IntQueue	&_in;
		IntQueue	&_out;
		PassTheBuck(const PassTheBuck&); ///< Prevent Usage
		PassTheBuck &operator=(const PassTheBuck&); ///< Prevent Usage
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	PassTheBuck::IntQueue	in(30), out;
	PassTheBuck		p1(in, out), p2(in, out), p3(in, out), p4(in, out), p5(in, out);
	PassTheBuck		p6(in, out), p7(in, out), p8(in, out), p9(in, out), p10(in, out);
	PassTheBuck		p11(in, out), p12(in, out), p13(in, out), p14(in, out), p15(in, out);
	PassTheBuck		p16(in, out), p17(in, out), p18(in, out), p19(in, out), p20(in, out);
	PassTheBuck		*threads[]= {
						&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10,
						&p11, &p12, &p13, &p14, &p15, &p16, &p17, &p18, &p19, &p20
					};
	int				fullCount= 0, emptyCount= 0;

	for(int value= 0; value < TestIterations; ++value) {
		if(in.empty()) {
			emptyCount++;
		}
		if(in.full()) {
			fullCount++;
		}
		in.enqueue(value);
		//printf("in.size()=%d full=%s\n", in.size(), in.full() ? "true" : "false");
	}
	for(unsigned int thread= 0; thread < sizeof(threads)/sizeof(threads[0]); ++thread) {
		threads[thread]->done();
	}
	printf("in queue empty %d times and full %d times\n", emptyCount, fullCount);
	while(in.size() > 0) {
		printf("Queue not empty\n");
		exec::Thread::sleep(5, exec::Thread::Milliseconds);
	}
	dotest( in.empty() );
	dotest( !in.full() );
	dotest( in.size() == 0 );
	in.close();
	for(unsigned int thread= 0; thread < sizeof(threads)/sizeof(threads[0]); ++thread) {
		threads[thread]->join();
	}
	return 0;
}
