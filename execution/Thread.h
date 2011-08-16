#ifndef __ExecutionThread_h__
#define __ExecutionThread_h__

#include <vector>
#include <pthread.h>
#include "MessageException.h"
#include "ExecutionRWLock.h"

namespace exec {
	
	class ThreadId {
	public:
		static ThreadId current();
		ThreadId();
		ThreadId(pthread_t thread);
		ThreadId(const ThreadId &thread);
		virtual ~ThreadId();
		ThreadId &operator=(const ThreadId &thread);
		bool operator==(const ThreadId &thread);
		bool operator!=(const ThreadId &thread);
		pthread_t thread();
		void *join();
		template<class T>
		T &instance();
		bool managed();
	protected:
		void exit(void *value= NULL);
		void manage();
	private:
		typedef std::vector<ThreadId*>	List;
		pthread_t	_id;
		static List &_managed();
		static void _destructor(void *me);
		static RWLock &_lock();
	};
	
	class Thread : public ThreadId {
	public:
		enum ThreadTerminationAction {
			KeepAroundAfterFinish,
			DeleteOnFinish
		};
		Thread(ThreadTerminationAction action= DeleteOnFinish);
		virtual ~Thread();
		void start();
	protected:
		virtual void *run();
	private:
		ThreadTerminationAction	_action;
		static void *_run(void *me);
		static pthread_t _create();
		Thread(const Thread&); ///< Prevent usage
		Thread &operator=(const Thread&); ///< Prevent usage
	};

	inline ThreadId ThreadId::current() {
		return ThreadId(pthread_self());
	}
	inline ThreadId::ThreadId()
		:_id() {
	}
	inline ThreadId::ThreadId(const ThreadId &theThread)
		:_id(theThread._id) {
	}
	inline ThreadId::~ThreadId() {
		try {
			RWLock::Locker	lock(_lock(), RWLock::Write);
			List::iterator	found= _managed().begin();
			
			while( (found != _managed().end()) && (*found != this) ) {
				++found;
			}
			if(found != _managed().end()) {
				_managed().erase(found);
			}
		} catch(const std::exception&) {
			// squelch exceptions in destructors
		}
	}
	inline ThreadId &ThreadId::operator=(const ThreadId &theThread) {
		_id= theThread._id;
		return *this;
	}
	inline bool ThreadId::operator==(const ThreadId &theThread) {
		return 0 != pthread_equal(_id, theThread._id);
	}
	inline bool ThreadId::operator!=(const ThreadId &theThread) {
		return 0 == pthread_equal(_id, theThread._id);
	}
	inline pthread_t ThreadId::thread() {
		return _id;
	}
	inline void *ThreadId::join() {
		void	*result= NULL;

		AssertCodeMessageException(pthread_join(_id, &result));
		return result;
	}
	template<class T>
	inline T &ThreadId::instance() {
		RWLock::Locker	lock(_lock(), RWLock::Read);
		List::iterator	found= _managed().begin();
		
		while( (found != _managed().end()) && (*found != this) ) {
			++found;
		}
		if(_managed().end() == found) {
			ThrowMessageException("Can't get instance of unmanaged thread");
		}
		return dynamic_cast<T&>(**found);
	}
	bool ThreadId::managed() {
		RWLock::Locker	lock(_lock(), RWLock::Read);
		List::iterator	found= _managed().begin();
		
		while( (found != _managed().end()) && (*found != this) ) {
			++found;
		}
		return (_managed().end() != found);
	}
	inline ThreadId::ThreadId(pthread_t theThread)
	:_id(theThread) {
	}
	inline void ThreadId::exit(void *value) {
		pthread_exit(value);
	}
	inline void ThreadId::manage() {
		RWLock::Locker	lock(_lock(), RWLock::Write);
		List::iterator	found= _managed().begin();
		
		while( (found != _managed().end()) && (*found != this) ) {
			++found;
		}
		if(found == _managed().end()) {
			_managed().push_back(this);
		}
	}
	inline ThreadId::List &ThreadId::_managed() {
		static List	threads;
		
		return threads;
	}
	inline void ThreadId::_destructor(void *me) {
		delete reinterpret_cast<ThreadId*>(me);
	}
	inline RWLock &ThreadId::_lock() {
		static RWLock	lock;
		
		return lock;
	}
	inline Thread::Thread(ThreadTerminationAction action)
	:ThreadId(), _action(action) {
	}
	inline Thread::~Thread() {
	}
	inline void Thread::start() {
		*reinterpret_cast<ThreadId*>(this)= ThreadId(_create());
		manage();
	}
	inline void *Thread::run() {
		return NULL;
	}
	inline void *Thread::_run(void *me) {
		void	*result= NULL;
		
		try {
			Thread	*theThread= reinterpret_cast<Thread*>(me);
			
			result= theThread->run();
			if(DeleteOnFinish == theThread->_action) {
				delete theThread;
			}
		} catch(const std::exception&) {
			// squelch exceptions in destructors
		}
		return result;
	}
	inline pthread_t Thread::_create() {
		pthread_t	tid;
		
		AssertCodeMessageException(pthread_create(&tid, NULL, _run, this));
		return tid;
	}
};

#endif // __ExecutionThread_h__
