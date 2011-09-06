#include "os/RWLock.h"
#include "os/Thread.h"
#include <stdio.h>

class Test : public exec::Thread {
	public:
		Test(exec::RWLock &lock, int period, exec::RWLock::ReadWrite rw)
			:exec::Thread(exec::Thread::KeepAroundAfterFinish), _lock(lock), _period(period), _rw(rw) {
			start();
		}
		~Test() {}
	protected:
		virtual void *run() {
			for(int i=0; i < 100; ++i) {
				exec::RWLock::Locker	locker(_lock, _rw);

				printf("> %s period %d\n", exec::RWLock::Read == _rw ? "Reading" : "Writing", _period);
				sleep(_period * 0.010); // sleep for period * 10 ms
				printf("< %s period %d\n", exec::RWLock::Read == _rw ? "Reading" : "Writing", _period);
			}
			return NULL;
		}
	private:
		exec::RWLock			&_lock;
		int						_period;
		exec::RWLock::ReadWrite	_rw;
};

int main(const int /*argc*/, const char * const /*argv*/[]) {
	exec::RWLock	lock;
	Test			r1(lock, 1, exec::RWLock::Read);
	Test			r2(lock, 2, exec::RWLock::Read);
	Test			w1(lock, 1, exec::RWLock::Write);
	Test			r3(lock, 3, exec::RWLock::Read);
	Test			r4(lock, 4, exec::RWLock::Read);
	Test			w2(lock, 2, exec::RWLock::Write);

	r1.join();
	r2.join();
	w1.join();
	r3.join();
	r4.join();
	w2.join();
	return 0;
}
