#include "os/Thread.h"
#include <stdio.h>

class Printer : public exec::Thread {
	public:
		Printer(const char *name)
			:Thread(KeepAroundAfterFinish), _name(name) {start();}
		virtual ~Printer() {}
	protected:
		virtual void *run() {
			for(int i=0; i < 100; ++i) {
				printf("%s: %d\n", _name, i);
				sleep(0.00001 /* 10 microseconds */ );
			}
			return NULL;
		}
	private:
		const char	*_name;
		Printer(const Printer&); ///< Prevent Usage
		Printer &operator=(const Printer&); ///< Prevent Usage
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	Printer	out("out"), err("err");

	out.join();
	err.join();
	return 0;
}
