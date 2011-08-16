#ifndef __ExecutionQueue_h__
#define __ExecutionQueue_h__

#include <vector>
#include <limits>
#include <pthread.h>
#include "MessageException.h"
#include "ExecutionMutex.h"
#include "ExecutionSignal.h"

namespace exec {

	template<class T>
	class Queue {
	public:
		Queue(int max=0, int reserve= 0);
		~Queue();
		bool empty();
		bool full();
		int size();
		Queue &enqueue(const T &value);
		bool enqueue(const T &value, double timeoutInSeconds);
		T dequeue();
		bool dequeue(T &value, double timeoutInSeconds);
		void close();
	private:
		typedef std::vector<T>	List;
		Mutex	_lock;
		Signal	_full;
		Signal	_empty;
		List	_queue;
		int		_max;
		Queue(const Queue&); ///< prevent usage
		Queue &operator=(const Queue&); ///< prevent usage
	};

	template<class T>
	inline Queue<T>::Queue(int max, int reserve)
	:_lock(), _full(), _empty(), _queue(), _max(0 == max ? std::numeric_limits<int>::max() : max) {
		if(reserve > 0) {
			_queue.reserve(reserve);
		} else if(max > 0) {
			_queue.reserve(max);
		}
	}
	template<class T>
	inline Queue<T>::~Queue() {
	}
	template<class T>
	inline bool Queue<T>::empty() {
		Mutex::Locker	lock(_lock);
		
		AssertMessageException(-1 != _max);
		return _queue.size() <= 0;
	}
	template<class T>
	inline bool Queue<T>::full() {
		Mutex::Locker	lock(_lock);
		
		AssertMessageException(-1 != _max);
		return _queue.size() >= _max;
	}
	template<class T>
	inline int Queue<T>::size() {
		Mutex::Locker	lock(_lock);
		
		AssertMessageException(-1 != _max);
		return _queue.size();
	}
	template<class T>
	inline Queue<T> &Queue<T>::enqueue(const T &value) {
		Mutex::Locker	lock(_lock);
		
		while( (static_cast<int>(_queue.size()) >= _max) && (-1 != _max) ) {
			_full.wait(_lock);
		}
		AssertMessageException(-1 != _max);
		_queue.insert(_queue.begin(), value);
		_empty.broadcast();
		return *this;
	}
	template<class T>
	inline bool Queue<T>::enqueue(const T &value, double timeoutInSeconds) {
		struct timeval	now;
		struct timespec	timeoutAt;
		
		AssertCodeMessageException(gettimeofday(&now, NULL));
		TIMEVAL_TO_TIMESPEC(&now, &timeoutAt);
		util::add(timeoutAt, timeoutInSeconds);

		Mutex::Locker	lock(_lock);
		
		while( (static_cast<int>(_queue.size()) >= _max) && (-1 != _max) ) {
			if(!_full.wait(_lock, timeoutAt)) {
				return false;
			} else {
				AssertMessageException(-1 != _max);
			}
		}
		AssertMessageException(-1 != _max);
		_queue.insert(_queue.begin(), value);
		_empty.broadcast();
		return true;
	}
	template<class T>
	inline T Queue<T>::dequeue() {
		Mutex::Locker	lock(_lock);
		
		while( (_queue.size() == 0) && (-1 != _max) ) {
			_empty.wait(_lock);
		}
		AssertMessageException(-1 != _max);
		
		T	value= _queue.back();
		
		_queue.pop_back();
		_full.broadcast();
		return value;
	}
	template<class T>
	inline bool Queue<T>::dequeue(T &value, double timeoutInSeconds) {
		struct timeval	now;
		struct timespec	timeoutAt;
		
		AssertCodeMessageException(gettimeofday(&now, NULL));
		TIMEVAL_TO_TIMESPEC(&now, &timeoutAt);
		util::add(timeoutAt, timeoutInSeconds);

		Mutex::Locker	lock(_lock);
		
		while( (_queue.size() == 0) && (-1 != _max) ) {
			if(!_empty.wait(_lock, timeoutAt)) {
				return false;
			} else {
				AssertMessageException(-1 != _max);
			}
		}
		AssertMessageException(-1 != _max);
		
		T	value= _queue.back();
		
		_queue.pop_back();
		_full.broadcast();
		return true;
	}
	template<class T>
	inline void Queue<T>::close() {
		_max= -1;
		_empty.broadcast();
		_full.broadcast();
		
	}
	
};

#endif // __ExecutionQueue_h__
