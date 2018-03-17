#include "os/Thread.h"
#include "os/DateTime.h"
#include <stdio.h>

class Printer : public exec::Thread {
	public:
		Printer(const char *name, bool d = false)
			:Thread(d ? DeleteOnFinish : KeepAroundAfterFinish), _name(name) {start();}
		virtual ~Printer() {}
	protected:
		virtual void *run() {
			for(int i=0; i < 6000; ++i) {
				printf("%s: %d\n", _name, i);
				dt::DateTime start;
				
				sleep(0.000000000000045, Years); // 10 microseconds
				sleep(0.000000000016534, Weeks); // 10 microseconds
				sleep(0.000000000115741, Days); // 10 microseconds
				sleep(0.000000002777778, Hours); // 10 microseconds
				sleep(0.000000166666667, Minutes); // 10 microseconds
				sleep(0.00001, Seconds); // 10 microseconds
				sleep(0.01, Milliseconds); // 10 microseconds
				sleep(10, Microseconds); // 10 microseconds
				double duration = dt::DateTime() - start;
				if (duration > 20 * 80 * 1000 * 1000) {
					printf("FAILED: Expecting 80 us, but we have %0.3f us\n", duration * 1000 * 1000);
				}
			}
			throw std::exception();
			return NULL;
		}
	private:
		const char	*_name;
		Printer(const Printer&); ///< Prevent Usage
		Printer &operator=(const Printer&); ///< Prevent Usage
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	Printer	out("out"), *err = new Printer("err");

	err->join();
	out.join();
	return 0;
}
