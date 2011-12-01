#ifndef __Thread_h__
#define __Thread_h__

#include "Exception.h"
#include <pthread.h>
#include <exception>
#include <unistd.h>

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

/// Execution related items.
namespace exec {

	/// Wrapper around a system Thread ID.
	class ThreadId {
		public:
			typedef ::pthread_t	SystemID; ///< Abstract System ID
			/// Get current thread ID
			static ThreadId current();
			enum TimeUnits {
				Years, Weeks, Days, Hours, Minutes, Seconds, Milliseconds, Microseconds
			};
			/// Sleep current thread by n seconds
			static void sleep(double time, TimeUnits units= Seconds);
			/// No thread
			ThreadId();
			/// Wrap a system thread ID
			ThreadId(SystemID thread);
			/// Copy constructor
			ThreadId(const ThreadId &thread);
			/// Does nothing
			virtual ~ThreadId();
			/// Assignment
			ThreadId &operator=(const ThreadId &thread);
			/// Compare the thread IDs
			bool operator==(const ThreadId &thread);
			/// Compare the thread IDs
			bool operator!=(const ThreadId &thread);
			/// Get the System ID of this thread
			SystemID thread();
			/// Wait for the thread to exit
			void *join();
		protected:
			/// Called from run() in subclass, sets the return value for join and returns early.
			void exit(void *value= NULL);
		private:
			SystemID	_id;	///< The System ID
	};

	/// Parent class for user defined threads.
	class Thread : public ThreadId {
		public:
			/// What to do when the thread is done.
			enum ThreadTerminationAction {
				KeepAroundAfterFinish,	///< Do nothing whent run() completes
				DeleteOnFinish			///< Call delete this; when run() completes
			};
			/// Called in ctor list of subclass.
			Thread(ThreadTerminationAction action= DeleteOnFinish);
			/// Does nothing
			virtual ~Thread();
			/// Kicks off the running of the thread.
			void start();
		protected:
			/// Only method you must override, the thread execution method.
			virtual void *run()= 0;
			/// If an exception is thrown during run(), what to do.
			virtual void *handle(const std::exception &exception, void *result);
		private:
			ThreadTerminationAction	_action;	///< What to do when run() is done.
			static void *_run(void *me);		///< The OS function that gets called.
			SystemID _create();					///< Create the system thread.
			Thread(const Thread&); ///< Prevent usage
			Thread &operator=(const Thread&); ///< Prevent usage
	};

	/**
		@todo Test!
	*/
	inline ThreadId ThreadId::current() {trace_scope
		return ThreadId(::pthread_self());
	}
	/**
		@todo Test!
	*/
	inline ThreadId::ThreadId()
		:_id() {trace_scope
	}
	/**
		@todo Test!
	*/
	inline ThreadId::ThreadId(const ThreadId &theThread)
		:_id(theThread._id) {trace_scope
	}
	inline ThreadId::~ThreadId() {trace_scope
	}
	/**
		@todo Test!
	*/
	inline ThreadId &ThreadId::operator=(const ThreadId &theThread) {trace_scope
		_id= theThread._id;
		return *this;
	}
	/**
		@todo Test!
	*/
	inline bool ThreadId::operator==(const ThreadId &theThread) {trace_scope
		return trace_bool(0 != ::pthread_equal(_id, theThread._id));
	}
	/**
		@todo Test!
	*/
	inline bool ThreadId::operator!=(const ThreadId &theThread) {trace_scope
		return trace_bool(0 == ::pthread_equal(_id, theThread._id));
	}
	/**
		@todo Test!
	*/
	inline ThreadId::SystemID ThreadId::thread() {trace_scope
		return _id;
	}
	inline void *ThreadId::join() {trace_scope
		void	*result= NULL;

		AssertCodeMessageException(::pthread_join(_id, &result));
		return result;
	}
	/**
		@todo Test!
	*/
	inline ThreadId::ThreadId(SystemID theThread)
		:_id(theThread) {trace_scope
	}
	/**
		@todo Test!
	*/
	inline void ThreadId::exit(void *value) {trace_scope
		::pthread_exit(value);
	}
	inline void ThreadId::sleep(double time, TimeUnits units) {trace_scope
		static const double 		FourBillionMicrosecondsInSeconds= 4000.0;
		double	seconds;
		
		switch(units) {
			case Years:				seconds= time * 365.2525 * 24.0 * 60.0 * 60.0;	break;
			case Weeks:				seconds= time * 7.0 * 24.0 * 60.0 * 60.0;		break;
			case Days:				seconds= time * 24.0 * 60.0 * 60.0;				break;
			case Hours:				seconds= time * 60.0 * 60.0;					break;
			case Minutes:			seconds= time * 60.0;							break;
			case Seconds: default:	seconds= time;									break;
			case Milliseconds:		seconds= time / 1000.0;							break;
			case Microseconds:		seconds= time / 1000.0 / 1000.0;				break;
		}
		//printf("time=%f units=%d seconds=%f\n", time, static_cast<int>(units), seconds);
		if(seconds > FourBillionMicrosecondsInSeconds) {
			static const unsigned int	MaxTries= 10;
			unsigned int				integerSeconds= static_cast<unsigned int>(seconds);
			unsigned int				tries= 0;

			while( trace_bool(tries < MaxTries) && trace_bool(integerSeconds= ::sleep(integerSeconds)) ) {
				++tries;
			}
		} else {
			unsigned long	microseconds= static_cast<unsigned long>(seconds * 1000.0 * 1000.0);

			::usleep(microseconds);
		}
	}
	inline Thread::Thread(ThreadTerminationAction action)
		:ThreadId(), _action(action) {trace_scope
	}
	inline Thread::~Thread() {trace_scope
	}
	inline void Thread::start() {trace_scope
		*reinterpret_cast<ThreadId*>(this)= ThreadId(_create());
	}
	inline void *Thread::_run(void *me) {trace_scope
		void	*result= NULL;
		Thread	*theThread= reinterpret_cast<Thread*>(me);

		try {
			result= theThread->run();
			if(DeleteOnFinish == theThread->_action) {
				delete theThread;
			}
		} catch(const std::exception &exception) {
			result= theThread->handle(exception, result);
		}
		return result;
	}
	/**
		@todo Test!
	*/
	inline void *Thread::handle(const std::exception &/*exception*/, void *result) {trace_scope
		return result;
	}
	inline Thread::SystemID Thread::_create() {trace_scope
		SystemID	tid;

		AssertCodeMessageException(::pthread_create(&tid, NULL, _run, this));
		return tid;
	}
}

#endif // __Thread_h__
