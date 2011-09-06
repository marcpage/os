#ifndef __ExecutionRWLock_h__
#define __ExecutionRWLock_h__

#include <vector>
#include <pthread.h>
#include "Exception.h"

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

namespace exec {

	/**
		@todo Document
		@todo Improve test to detect r/w conflict
	*/
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
		:_lock(lock) {trace_scope;
		_lock.lock(read);
	}
	inline RWLock::Locker::~Locker() {trace_scope;
		_lock.unlock();
	}
	inline RWLock::RWLock()
		:_lock() {trace_scope;
		AssertCodeMessageException(pthread_rwlock_init(&_lock, NULL));
	}
	inline RWLock::~RWLock() {trace_scope;
		int		error= pthread_rwlock_destroy(&_lock);
		void	*__unused__[]= {&error, &__unused__};
	}
	inline void RWLock::lock(ReadWrite read) {trace_scope;
		if(Read == read) {
			AssertCodeMessageException(pthread_rwlock_rdlock(&_lock));
		} else {
			AssertCodeMessageException(pthread_rwlock_wrlock(&_lock));
		}
	}
	inline void RWLock::unlock() {trace_scope;
		AssertCodeMessageException(pthread_rwlock_unlock(&_lock));
	}

};

#endif // __ExecutionRWLock_h__
