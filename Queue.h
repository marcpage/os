#ifndef __Queue_h__
#define __Queue_h__

#include <vector>
#include <limits>
#include <mutex>
#include <condition_variable>
#include "Exception.h"

#define AssertQueueNotClosed	if(-1 == _max) {throw Closed("Queue Already Closed", __FILE__, __LINE__);} else msg::noop()

namespace exec {

/**
	@todo Document
	@todo Test
*/
	template<class T>
	class Queue {
	public:
		class Closed : public msg::Exception {
		public:
			/// Get a message
			Closed(const char *message, const char *file= NULL, int line= 0) throw();
			/// Get a message
			Closed(const std::string &message, const char *file= NULL, int line= 0) throw();
			/// Copy constructor
			Closed(const Closed &other);
			/// destructs _message
			virtual ~Closed() throw();
		};
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
		std::mutex	_lock;
		std::condition_variable		_full;
		std::condition_variable		_empty;
		List		_queue;
		int			_max;
		Queue(const Queue&); ///< prevent usage
		Queue &operator=(const Queue&); ///< prevent usage
	};

	template<class T>
	inline Queue<T>::Closed::Closed(const char *message, const char *file, int line) throw()
			:msg::Exception(message, file, line) {}
	template<class T>
	inline Queue<T>::Closed::Closed(const std::string &message, const char *file, int line) throw()
			:msg::Exception(message, file, line) {}
	template<class T>
	inline Queue<T>::Closed::Closed(const Closed &other)
			:msg::Exception(other) {}
	template<class T>
	inline  Queue<T>::Closed::~Closed() throw() {}
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
		std::lock_guard<std::mutex>	lock(_lock);

		AssertQueueNotClosed;
		return _queue.size() <= 0;
	}
	template<class T>
	inline bool Queue<T>::full() {
		std::lock_guard<std::mutex>	lock(_lock);

		AssertQueueNotClosed;
		AssertMessageException(_max > 0);
		return _queue.size() >= static_cast<unsigned int>(_max);
	}
	template<class T>
	inline int Queue<T>::size() {
		std::lock_guard<std::mutex>	lock(_lock);

		AssertQueueNotClosed;
		return _queue.size();
	}
	template<class T>
	inline Queue<T> &Queue<T>::enqueue(const T &value) {
		std::unique_lock<std::mutex>	lock(_lock);

		while( (static_cast<int>(_queue.size()) >= _max) && (-1 != _max) ) {
			_full.wait(lock);
		}
		AssertQueueNotClosed;
		_queue.insert(_queue.begin(), value);
		_empty.notify_all();
		return *this;
	}
	template<class T>
	inline bool Queue<T>::enqueue(const T &value, double timeoutInSeconds) {
		std::unique_lock<std::mutex>	lock(_lock);

		while( (static_cast<int>(_queue.size()) >= _max) && (-1 != _max) ) {
			if(!_full.wait_for(lock, std::chrono::seconds(uint64_t(timeoutInSeconds) * 1000 * 1000 * 1000))) {
				return false;
			} else {
				AssertQueueNotClosed;
			}
		}
		AssertQueueNotClosed;
		_queue.insert(_queue.begin(), value);
		_empty.notify_all();
		return true;
	}
	template<class T>
	inline T Queue<T>::dequeue() {
		std::unique_lock<std::mutex>	lock(_lock);

		while( (_queue.size() == 0) && (-1 != _max) ) {
			_empty.wait(lock);
		}
		AssertQueueNotClosed;

		T	value= _queue.back();

		_queue.pop_back();
		_full.notify_all();
		return value;
	}
	template<class T>
	inline bool Queue<T>::dequeue(T &value, double timeoutInSeconds) {
		std::unique_lock<std::mutex>	lock(_lock);

		while( (_queue.size() == 0) && (-1 != _max) ) {
			if(!_empty.wait_for(lock, std::chrono::nanoseconds(uint64_t(timeoutInSeconds) * 1000 * 1000 * 1000))) {
				return false;
			} else {
				AssertQueueNotClosed;
			}
		}
		AssertQueueNotClosed;
		value= _queue.back();
		_queue.pop_back();
		_full.notify_all();
		return true;
	}
	template<class T>
	inline void Queue<T>::close() {
		_max= -1;
		_empty.notify_all();
		_full.notify_all();

	}

};

#endif // __Queue_h__
