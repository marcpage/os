#ifndef __ExecutionRWLock_h__
#define __ExecutionRWLock_h__

#include <vector>
#include <pthread.h>
#include "Exception.h"

namespace exec {

	class RWLock {
	public:
		enum ReadWrite {
			Read,
			Write
		};
		class Locker {
		public:
			Locker(RWLock &lock, ReadWrite read);
			~Locker();
		private:
			RWLock	&_lock;
			Locker(const Locker&); // prevent usage
			Locker &operator=(const Locker&); // prevent usage
		};
		RWLock();
		~RWLock();
		void lock(ReadWrite read);
		void unlock();
	private:
		pthread_rwlock_t	_lock;
		RWLock(const RWLock&); // prevent usage
		RWLock &operator=(const RWLock&); // prevent usage
	};

	inline RWLock::Locker::Locker(RWLock &lock, ReadWrite read)
		:_lock(lock) {
		_lock.lock(read);
	}
	inline RWLock::Locker::~Locker() {
		_lock.unlock();
	}
	inline RWLock::RWLock()
		:_lock() {
		AssertCodeMessageException(pthread_rwlock_init(&_lock, NULL));
	}
	inline RWLock::~RWLock() {
		int		error= pthread_rwlock_destroy(&_lock);
		void	*__unused__[]= {&error, &__unused__};
	}
	inline void RWLock::lock(ReadWrite read) {
		if(Read == read) {
			AssertCodeMessageException(pthread_rwlock_rdlock(&_lock));
		} else {
			AssertCodeMessageException(pthread_rwlock_wrlock(&_lock));
		}
	}
	inline void RWLock::unlock() {
		AssertCodeMessageException(pthread_rwlock_unlock(&_lock));
	}

};

#endif // __ExecutionRWLock_h__
