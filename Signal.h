#ifndef __ExecutionSignal_h__
#define __ExecutionSignal_h__

#include <vector>
#include <pthread.h>
#include "Exception.h"
#include "Mutex.h"
#include "DateTime.h"

namespace exec {

	/**
		@todo Document and test
	*/
	class Signal {
		public:
			Signal();
			~Signal();
			/// Wakes up all threads waiting on this Signal
			void broadcast();
			/// Wakes up one thread waiting on this Signal
			void signal();
			/// Unlocks the mutex and waits for the signal
			void wait(Mutex &mutex);
			/// Unlocks the mutex and waits for the signal or we reach a certain time
			bool wait(Mutex &mutex, const dt::DateTime &untilAbsTime);
			/// Unlocks the mutex and waits for the signal or we timeout
			bool wait(Mutex &mutex, double timeoutInSeconds);
		private:
			pthread_cond_t	_signal;	///< The signal
			Signal(const Signal&); ///< prevent usage
			Signal &operator=(const Signal&); ///< prevent usage
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
	inline bool Signal::wait(Mutex &mutex, const dt::DateTime &untilAbsTime) {
		struct timespec	timeoutAt;
		int				returnCode;

		returnCode= pthread_cond_timedwait(&_signal, mutex, &untilAbsTime.value(timeoutAt));
		if(ETIMEDOUT == returnCode) {
			return false;
		}
		AssertCodeMessageException(returnCode);
		return true;
	}
	inline bool Signal::wait(Mutex &mutex, double timeoutInSeconds) {
		dt::DateTime	now;
		struct timespec	timeoutAt;

		now+= timeoutInSeconds;
		wait(mutex, &now.value(timeoutAt));
	}
};

#endif // __ExecutionSignal_h__
