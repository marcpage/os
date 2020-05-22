#ifndef __Queue_h__
#define __Queue_h__

#include "Exception.h"
#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>

/// Helper macro that will throw an exception before accessing a closed queue
#define AssertQueueNotClosed                                                   \
  if (-1 == _max) {                                                            \
    throw Closed("Queue Already Closed", __FILE__, __LINE__);                  \
  } else                                                                       \
    msg::noop()

namespace exec {

/// A thread-safe queue of a given type
template <class T> class Queue {
public:
  /// Exception that is thrown when attempting to operate on a closed queue
  class Closed : public msg::Exception {
  public:
    /// Get a message
    explicit Closed(const char *message, const char *file = NULL,
                    int line = 0) throw();
    /// Get a message
    explicit Closed(const std::string &message, const char *file = NULL,
                    int line = 0) throw();
    /// Copy constructor
    Closed(const Closed &other);
    /// destructs _message
    virtual ~Closed() throw();
  };
  /** Create a new queue.
        @param max The maximum number of items in the queue. If enqueue is
     attempted, it blocks until there is room. default 0 which means there is no
     limit.
        @param reserve The guess at how many items will be in the queue at peak.
     default 0 which means no space reserved.
  */
  Queue(int max = 0, int reserve = 0);
  /// Destructor
  ~Queue();
  /** Is the queue empty.
        @return true if there are no elements in the queue.
  */
  bool empty();
  /** Is the queue full.
        @return true if any more elements added to the queue would block.
        @throws Closed if the queue has been closed
  */
  bool full();
  /** The number of elements in the queue.
        @return The count of all elements in the queue.
        @throws Closed if the queue has been closed
  */
  int size();
  /** Puts an element into the queue.
        This call will block if the queue is full until an element is dequeued.
        @param value The value to add to the queue.
        @return a reference to this queue
        @throws Closed if the queue has been closed
  */
  Queue &enqueue(const T &value);
  /** Puts an element into the queue.
        This call will block if the queue is full until an element is dequeued.
        @param value The value to add to the queue.
        @param timeoutInSeconds The number of seconds to wait if the queue is
     full.
        @return true if the element was successfully added, false if the
     operation timed out
        @throws Closed if the queue has been closed
  */
  bool enqueue(const T &value, double timeoutInSeconds);
  /** Retrieves that oldest value in the queue.
        @return The oldest value in the queue
        @throws Closed if the queue has been closed
  */
  T dequeue();
  /** Retrieves that oldest value in the queue.
        @param value receives the value from the queue
        @param timeoutInSeconds The number of seconds to wait if the queue is
     empty.
        @return The oldest value in the queue
        @throws Closed if the queue has been closed
  */
  bool dequeue(T &value, double timeoutInSeconds);
  /// Closes the queue, preventing further operations on the queue.
  void close();

private:
  typedef std::deque<T> List;      ///< A list of elements
  std::mutex _lock;                ///< Protection on _queue
  std::condition_variable _full;   ///< Used to wait if the queue is full
  std::condition_variable _empty;  ///< Used to wait if the queue is empty
  List _queue;                     ///< The queue of elements
  int _max;                        ///< The maximum number of elements
  Queue(const Queue &);            ///< prevent usage
  Queue &operator=(const Queue &); ///< prevent usage
};

template <class T>
inline Queue<T>::Closed::Closed(const char *message, const char *file,
                                int line) throw()
    : msg::Exception(message, file, line) {}
template <class T>
inline Queue<T>::Closed::Closed(const std::string &message, const char *file,
                                int line) throw()
    : msg::Exception(message, file, line) {}
template <class T>
inline Queue<T>::Closed::Closed(const Closed &other) : msg::Exception(other) {}
template <class T> inline Queue<T>::Closed::~Closed() throw() {}
template <class T>
inline Queue<T>::Queue(int max, int reserve)
    : _lock(), _full(), _empty(), _queue(),
      _max(0 == max ? std::numeric_limits<int>::max() : max) {
  if (reserve > 0) {
    //_queue.reserve(reserve);
  } else if (max > 0) {
    //_queue.reserve(max);
  }
}
template <class T> inline Queue<T>::~Queue() {}
template <class T> inline bool Queue<T>::empty() {
  std::lock_guard<std::mutex> lock(_lock);

  AssertQueueNotClosed;
  return _queue.size() == 0;
}
template <class T> inline bool Queue<T>::full() {
  std::lock_guard<std::mutex> lock(_lock);

  AssertQueueNotClosed;
  AssertMessageException(_max > 0);
  return _queue.size() >= static_cast<unsigned int>(_max);
}
template <class T> inline int Queue<T>::size() {
  std::lock_guard<std::mutex> lock(_lock);

  AssertQueueNotClosed;
  return _queue.size();
}
template <class T> inline Queue<T> &Queue<T>::enqueue(const T &value) {
  std::unique_lock<std::mutex> lock(_lock);

  while ((static_cast<int>(_queue.size()) >= _max) && (-1 != _max)) {
    _full.wait(lock);
  }
  AssertQueueNotClosed;
  //_queue.insert(_queue.begin(), value);
  _queue.push_front(value);
  _empty.notify_all();
  return *this;
}
/**
        @todo decrement timeoutInSeconds for contention on enqueues
*/
template <class T>
inline bool Queue<T>::enqueue(const T &value, double timeoutInSeconds) {
  std::unique_lock<std::mutex> lock(_lock);

  while ((static_cast<int>(_queue.size()) >= _max) && (-1 != _max)) {
    if (std::cv_status::timeout ==
        _full.wait_for(lock, std::chrono::seconds(uint64_t(
                                 timeoutInSeconds * 1000 * 1000 * 1000)))) {
      return false;
    } else {
      AssertQueueNotClosed;
    }
  }
  AssertQueueNotClosed;
  //_queue.insert(_queue.begin(), value);
  _queue.push_front(value);
  _empty.notify_all();
  return true;
}
template <class T> inline T Queue<T>::dequeue() {
  std::unique_lock<std::mutex> lock(_lock);

  while ((_queue.size() == 0) && (-1 != _max)) {
    _empty.wait(lock);
  }
  AssertQueueNotClosed;

  T value = _queue.back();

  _queue.pop_back();
  _full.notify_all();
  return value;
}
/**
        @todo decrement timeoutInSeconds for contention on enqueues
*/
template <class T>
inline bool Queue<T>::dequeue(T &value, double timeoutInSeconds) {
  std::unique_lock<std::mutex> lock(_lock);

  while ((_queue.size() == 0) && (-1 != _max)) {
    if (std::cv_status::timeout ==
        _empty.wait_for(lock, std::chrono::nanoseconds(uint64_t(
                                  timeoutInSeconds * 1000 * 1000 * 1000)))) {
      return false;
    } else {
      AssertQueueNotClosed;
    }
  }
  AssertQueueNotClosed;
  value = _queue.back();
  _queue.pop_back();
  _full.notify_all();
  return true;
}
template <class T> inline void Queue<T>::close() {
  _max = -1;
  _empty.notify_all();
  _full.notify_all();
}

#undef AssertQueueNotClosed // this macro is not valid outside the scope of this
                            // file

}; // namespace exec

#endif // __Queue_h__
