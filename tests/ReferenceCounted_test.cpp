#include "os/Thread.h"
#include "os/ReferenceCounted.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

class Tester : public exec::Thread {
	public:
		typedef exec::ReferenceCounted::Ptr<exec::ReferenceCounted>	Ptr;
		Tester(const Ptr &ptr)
			:exec::Thread(KeepAroundAfterFinish), _value(ptr) {
			start();
			//printf("Started\n");
		}
		virtual ~Tester() {}
	protected:
		virtual void *run() {
			Ptr	bunch[50];
			
			for(int set= 0; set < 50; ++set) {
				//printf("set=%d\n",set);
				for(unsigned int index= 0; index < sizeof(bunch)/sizeof(bunch[0]); ++index) {
					//printf("first assign=%d\n",index);
					bunch[index]= _value;
				}
				for(unsigned int index= 0; index < sizeof(bunch)/sizeof(bunch[0]); ++index) {
					//printf("clear=%d\n",index);
					bunch[index].assign(NULL);
				}
				dotest(_value->references() != 0);
				for(unsigned int index= 0; index < sizeof(bunch)/sizeof(bunch[0]); ++index) {
					//printf("last assign=%d\n",index);
					bunch[index]= Ptr(_value);
				}
				_value= _value;
				//printf("starting sleep\n");
				sleep(10, Microseconds);
				//printf("ended sleep\n");
			}
			return NULL;
		}
	private:
		Ptr	_value;
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	exec::ReferenceCounted	*item= new exec::ReferenceCounted();
	
	do	{
		Tester::Ptr	test(item); // retains, references goes to 2
		Tester		t1(test), t2(test), t3(test), t4(test), t5(test), t6(test), t7(test);
		//printf("All Started\n");
		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		//printf("All Done %d", static_cast<int>(test->references()));
		dotest(test->references() == 1 + 1 + 7); // item, test and each t#
	} while(false);
	dotest(item->references() == 1);
	item->release();
	return 0;
}
