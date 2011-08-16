#ifndef __ExecutionAtomic_h__
#define __ExecutionAtomic_h__

#include <stdint.h>
#include "ExecutionMutex.h"

#define MACOSX	(__APPLE_CC__ || __APPLE__)

#if MACOSX
	#include <libkern/OSAtomic.h>
	#define ExecutionAtomicInitialize
#else
	#define ExecutionAtomicInitialize exec::Mutex *exec::AtomicNumber::_mutex= NULL
#endif

namespace exec {

	class AtomicNumber {
		public:
			static void cleanup();
			AtomicNumber(int32_t value= 0);
			~AtomicNumber();
			int32_t value() const;
			int32_t valueBeforeIncrement(int32_t amount= 1);
			int32_t valueBeforeDecrement(int32_t amount= 1);
			int32_t valueAfterIncrement(int32_t amount= 1);
			int32_t valueAfterDecrement(int32_t amount= 1);
			operator int32_t() const;
			// we do not have prefix increment/decrement since we can't really return a reference to ourselves
			AtomicNumber operator++(int);
			AtomicNumber operator--(int);
		private:
			volatile int32_t	_number;
#if MACOSX
#else
			static Mutex		*_mutex;
			static Mutex &_globalMutex();
#endif
	};

	inline AtomicNumber::AtomicNumber(int32_t initialValue)
		:_number(initialValue) {
	}
	inline AtomicNumber::~AtomicNumber() {
	}
	inline int32_t AtomicNumber::value() const {
		return _number;
	}
	inline int32_t AtomicNumber::valueBeforeIncrement(int32_t amount) {
#if MACOSX
		return OSAtomicAdd32Barrier(amount, &_number) - amount;
#else
		Mutex::Locker(_globalMutex());
		
		int32_t	original= _number;
		
		_number+= amount;
		return original;
#endif
	}
	inline int32_t AtomicNumber::valueBeforeDecrement(int32_t amount) {
#if MACOSX
		return OSAtomicAdd32Barrier(-1*amount, &_number) + amount;
#else
		Mutex::Locker(_globalMutex());
		
		int32_t	original= _number;
		
		_number-= amount;
		return original;
#endif
	}
	inline int32_t AtomicNumber::valueAfterIncrement(int32_t amount) {
#if MACOSX
		return OSAtomicAdd32Barrier(amount, &_number);
#else
		Mutex::Locker(_globalMutex());
		
		return _number+= amount;
#endif
	}
	inline int32_t AtomicNumber::valueAfterDecrement(int32_t amount) {
#if MACOSX
		return OSAtomicAdd32Barrier(-1*amount, &_number);
#else
		Mutex::Locker(_globalMutex());
		
		return _number-= amount;
#endif
	}
	inline AtomicNumber::operator int32_t() const {
		return value();
	}
	inline AtomicNumber AtomicNumber::operator++(int) {
		return valueBeforeIncrement();
	}
	inline AtomicNumber AtomicNumber::operator--(int) {
		return valueBeforeDecrement();
	}
#if !MACOSX
	Mutex &AtomicNumber::_globalMutex() {
		if(NULL == _mutex) {
			_mutex= new Mutex();
		}
		return *_mutex;
	}
#endif
	inline void AtomicNumber::cleanup() {
#if MACOSX
#else
		if(NULL != _mutex) {
			delete _mutex;
		}
#endif
	}

}

#endif __ExecutionAtomic_h__
