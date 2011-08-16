#ifndef __ExecutionMutex_h__
#define __ExecutionMutex_h__

#include <vector>
#include <pthread.h>
#include "MessageException.h"

namespace exec {
	
	class Mutex {
	public:
		class Locker {
		public:
			Locker(Mutex &mutex);
			~Locker();
		private:
			Mutex	&_mutex;
			Locker(const Locker&); // prevent usage
			Locker &operator=(const Locker&); // prevent usage
		};
		Mutex();
		~Mutex();
		void lock();
		void unlock();
		operator pthread_mutex_t*();
	private:
		pthread_mutex_t	_mutex;
		Mutex(const Mutex&); // prevent usage
		Mutex &operator=(const Mutex&); // prevent usage
	};

	inline Mutex::Locker::Locker(Mutex &mutex)
	:_mutex(mutex) {
		_mutex.lock();
	}
	inline Mutex::Locker::~Locker() {
		_mutex.unlock();
	}
	inline Mutex::Mutex()
	:_mutex() {
		AssertCodeMessageException(pthread_mutex_init(&_mutex, NULL));
	}
	inline Mutex::~Mutex() {
		int		error= pthread_mutex_destroy(&_mutex);
		void	*__unused__[]= {&error, &__unused__};
	}
	inline void Mutex::lock() {
		AssertCodeMessageException(pthread_mutex_lock(&_mutex));
	}
	inline void Mutex::unlock() {
		AssertCodeMessageException(pthread_mutex_unlock(&_mutex));
	}
	inline Mutex::operator pthread_mutex_t*() {
		return &_mutex;
	}
};

#endif // __ExecutionMutex_h__
