#ifndef __ExecutionMutex_h__
#define __ExecutionMutex_h__

#include <pthread.h>
#include "Exception.h"

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

/// Utility Macro to work preprocessor magic, do not use directly
#define MUTEX_PASTER(x,y) __ ## x ## _ ## y
/// Creates a unique variable name (per line)
#define MUTEX_UNIQUE_VARIABLE_NAME(x,y)  MUTEX_PASTER(x,y)
/// Creates a lock/unlock section for a mutex
#define mutex_section(mutex) exec::Mutex::Locker MUTEX_UNIQUE_VARIABLE_NAME(auto_mutex_lock_unlock,__LINE__)(mutex)

namespace exec {

	/** A class to make using Mutexes easier.
	*/
	class Mutex {
	public:
		typedef ::pthread_mutex_t	SystemMutex;
		/** A utility class to automatically unlock a mutex at the end of a scope.
		*/
		class Locker {
		public:
			/// Locks and remembers the mutex
			Locker(Mutex &mutex);
			/// Unlocks the remembered mutex
			~Locker();
		private:
			Mutex	&_mutex; ///< The remembered mutex
			Locker(const Locker&); ///< prevent usage
			Locker &operator=(const Locker&); ///< prevent usage
		};
		/// Create a mutex.
		Mutex();
		/// Destroy the mutex.
		~Mutex();
		/// Lock the mutex.
		void lock();
		/// Unlocks the mutex.
		void unlock();
		/// Get the system mutex object.
		operator SystemMutex*();
	private:
		SystemMutex	_mutex; ///< The system mutex.
		Mutex(const Mutex&); ///< prevent usage
		Mutex &operator=(const Mutex&); ///< prevent usage
	};

	/** Remembers the mutex and locks it.
		@param mutex	The mutex to lock and remember.
	*/
	inline Mutex::Locker::Locker(Mutex &mutex)
	:_mutex(mutex) {trace_scope;
		_mutex.lock();
	}
	/** Unlocks the remembered mutex.
	*/
	inline Mutex::Locker::~Locker() {trace_scope;
		_mutex.unlock();
	}
	/** Creates a mutex.
		@throws msg::MessageException	If initing the mutex fails.
	*/
	inline Mutex::Mutex()
	:_mutex() {trace_scope;
		AssertCodeMessageException(pthread_mutex_init(&_mutex, NULL));
	}
	/** Destroys the mutex.
	*/
	inline Mutex::~Mutex() {trace_scope;
		int		error= pthread_mutex_destroy(&_mutex);
		void	*__unused__[]= {&error, &__unused__};
	}
	/** Prevents others from locking this mutex.
		If someone already has this mutex lock()'ed,
		@throws msg::MessageException	If locking the mutex fails.
	*/
	inline void Mutex::lock() {trace_scope;
		AssertCodeMessageException(pthread_mutex_lock(&_mutex));
	}
	/** Allows others to lock this mutex.
		If someone is already trying to lock() this mutex, they may be allowed to.
		@throws msg::MessageException	If unlocking the mutex fails.
	*/
	inline void Mutex::unlock() {trace_scope;
		AssertCodeMessageException(pthread_mutex_unlock(&_mutex));
	}
	/** Gets the system mutex behind the implementation.
		@return	System mutex used.
	*/
	inline Mutex::operator SystemMutex*() {trace_scope;
		return &_mutex;
	}
};

#endif // __ExecutionMutex_h__
