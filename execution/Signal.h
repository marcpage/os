#ifndef __ExecutionSignal_h__
#define __ExecutionSignal_h__

#include <vector>
#include <pthread.h>
#include "MessageException.h"
#include "ExecutionMutex.h"
#include "Utilities.h"

namespace exec {
	
	class Signal {
	public:
		Signal();
		~Signal();
		void broadcast();
		void signal();
		// Unlocks the mutex and waits for the signal
		void wait(Mutex &mutex);
		bool wait(Mutex &mutex, time_t untilAbsTime);
		bool wait(Mutex &mutex, double timeoutInSeconds);
	private:
		pthread_cond_t	_signal;
		Signal(const Signal&); // prevent usage
		Signal &operator=(const Signal&); // prevent usage
	};

	inline Signal::Signal()
	:_signal() {
		AssertCodeMessageException(pthread_cond_init(&_signal, NULL));
	}
	inline Signal::~Signal() {
		int		error= pthread_cond_destroy(&_signal);
		void	*__unused__[]= {&error, &__unused__};
	}
	inline void Signal::broadcast() {
		AssertCodeMessageException(pthread_cond_broadcast(&_signal));
	}
	inline void Signal::signal() {
		AssertCodeMessageException(pthread_cond_signal(&_signal));
	}
	inline void Signal::wait(Mutex &mutex) {
		AssertCodeMessageException(pthread_cond_wait(&_signal, mutex));
	}
	inline bool Signal::wait(Mutex &mutex, time_t untilAbsTime) {
		int	returnCode= pthread_cond_timedwait(&_signal, mutex, &timeoutAt);
		
		if(ETIMEDOUT == returnCode) {
			return false;
		}
		AssertCodeMessageException(returnCode);
		return true;
	}
	inline bool Signal::wait(Mutex &mutex, double timeoutInSeconds) {
		struct timeval	now;
		struct timespec	timeoutAt;
		
		AssertCodeMessageException(gettimeofday(&now, NULL));
		TIMEVAL_TO_TIMESPEC(&now, &timeoutAt);
		utils::add(timeoutAt, timeoutInSeconds);
		wait(mutex, timeoutAt);
	}
};

#endif // __ExecutionSignal_h__
