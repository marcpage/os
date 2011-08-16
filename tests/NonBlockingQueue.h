#ifndef __NonBlockingQueue_h__
#define __NonBlockingQueue_h__

#include <libkern/OSAtomic.h>
#include "MessageException.h"

#if sizeof(void*) == sizeof(uint64_t)
	#define CompareAndSwap	OSAtomicCompareAndSwap64Barrier
	#define AddressInt		uint64_t
#elif sizeof(void*) == sizeof(uint32_t)
	#define CompareAndSwap	OSAtomicCompareAndSwap32Barrier
	#define AddressInt		uint32_t
#else
	#error Unknown pointer size
#endif

namespace NonBlocking {

	template<class T>
	class Queue {
		public:
			Queue();
			~Queue();
			bool empty() const;
			Queue &enqueue(const T &value);
			T dequeue();
			void close();
		private:
			struct _Element {
				_Element(const T &initialValue):next(NULL), value(initialValue) {}
				_Element	*next;
				T			value;
			};
			typedef _Element	*_ElementPtr;
			_Element	*_head;
			bool		_active;
			T _dequeue();
	};

	template<class T>
	Queue<T>::Queue()
		:_head(NULL), _active(true) {
	}
	template<class T>
	Queue<T>::~Queue() {
	}
	template<class T>
	bool Queue<T>::empty() const {
		return NULL == _head;
	}
	template<class T>
	Queue &Queue<T>::enqueue(const T &value) {
		AssertMessageException(_active);
		_Element	*node= new _Element(value);
		
		do	{
			node->next= _head;
		} while(!CompareAndSwap(reinterpret_cast<AddressInt>(_head),
								reinterpret_cast<AddressInt>(node),
								reinterpret_cast<AddressInt*>(&_head)
				));
	}
	template<class T>
	T Queue<T>::dequeue() {
		AssertMessageException(_active);
		return _dequeue();
	}
	template<class T>
	void Queue<T>::close() {
		_active= false;
		while(!empty()) {
			_dequeue();
		}
	}
	template<class T>
	T Queue<T>::_dequeue() {
		_ElementPtr	*element;
		// fix this
		do	{
			element= &_head;
			ThrowMessageExceptionIfNULL(*element);
			while((*element)->next != NULL) {
				element= element->next;
			}
		} while(!CompareAndSwap
	}

}

#endif __NonBlockingQueue_h__
