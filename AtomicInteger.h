#ifndef __AtomicInteger_h__
#define __AtomicInteger_h__

#include <stdint.h>

#if defined(_WIN32) // http://predef.sourceforge.net/preos.html#sec25
	// http://msdn.microsoft.com/en-us/library/ms683614(v=vs.85).aspx
	#include <windows.h>
	#define WinInterlock 1
#elif 0 //defined(__APPLE__)
	// http://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man3/OSAtomicAdd32Barrier.3.html
	#include <libkern/OSAtomic.h>
	#define MacAtomic 1
#elif (__GNUC__ == 4) && (__GNUC_MINOR__ >=1) || (__GNUC__ > 4) // gcc 4.1 and later
	// http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html
	#define GCCBuiltInAtomic 1
#else
	#error Implement for this platform
#endif

namespace exec {

	/** An integer that can be incremented and decremented from multiple threads while maintaining correctness.
		@note we do not have prefix increment/decrement since we can't really return a reference to ourselves
	*/
	class AtomicInteger {
		public:
			/// The native int we are going to use
			typedef int32_t	NativeInt;
			/// Create a new int
			AtomicInteger(int32_t initialValue= 0);
			/// noop
			~AtomicInteger();
			/// Get a snapshot of the value
			NativeInt value() const;
			/// increment and get the previous value
			NativeInt valueBeforeIncrement(NativeInt amount= 1);
			/// decrement and get the previous value
			NativeInt valueBeforeDecrement(NativeInt amount= 1);
			/// increment and get the value after incrementing
			NativeInt valueAfterIncrement(NativeInt amount= 1);
			/// decrement and get the value after decrementing
			NativeInt valueAfterDecrement(NativeInt amount= 1);
			/// Be able to pass this class into places that take the native int
			operator NativeInt() const;
			/// Postfix increment, AtomicInteger++
			AtomicInteger operator++(int);
			/// Postfix decrement, AtomicInteger--
			AtomicInteger operator--(int);
		private:
		#if WinInterlock
			typedef LONG	AtomicInt32Type; ///< The type used for interlocking add
		#else
			typedef int32_t	AtomicInt32Type; ///< The type used for atomic add
		#endif
			volatile AtomicInt32Type	_number;	///< The number we are handling
			/// Core add functionality used by other methods.
			NativeInt _add(NativeInt amount);
	};

	/**
		@param initialValue	The value to start out with for this integer.
	*/
	inline AtomicInteger::AtomicInteger(NativeInt initialValue)
		:_number(initialValue) {;
	}
	/**
	*/
	inline AtomicInteger::~AtomicInteger() {;
	}
	/**
		@return The value of the integer
		@todo increment/decrement to get the value, or add 0
	*/
	inline AtomicInteger::NativeInt AtomicInteger::value() const {;
		return _number;
	}
	/**
		@param amount	The amount to add
		@return			The value before we added <code>amount</code>
	*/
	inline AtomicInteger::NativeInt AtomicInteger::valueBeforeIncrement(NativeInt amount) {;
		return _add(amount) - amount;
	}
	/**
		@param amount	The amount to subtract
		@return			The value before we subtracted <code>amount</code>
	*/
	inline AtomicInteger::NativeInt AtomicInteger::valueBeforeDecrement(NativeInt amount) {;
		return _add(-1*amount) + amount;
	}
	/**
		@param amount	The amount to add
		@return			The value after we added <code>amount</code>
	*/
	inline AtomicInteger::NativeInt AtomicInteger::valueAfterIncrement(NativeInt amount) {;
		return _add(amount);
	}
	/**
		@param amount	The amount to subtract
		@return			The value after we subtracted <code>amount</code>
	*/
	inline AtomicInteger::NativeInt AtomicInteger::valueAfterDecrement(NativeInt amount) {;
		return _add(-1*amount);
	}
	/** Used for typecasting and parameter passing.
		@return The value of the integer
	*/
	inline AtomicInteger::operator AtomicInteger::NativeInt() const {;
		return value();
	}
	/**
		@return the value before the increment.
	*/
	inline AtomicInteger AtomicInteger::operator++(int) {;
		return valueBeforeIncrement();
	}
	/**
		@return the value before the decrement.
	*/
	inline AtomicInteger AtomicInteger::operator--(int) {;
		return valueBeforeDecrement();
	}
	/**
		@param amount	The amount to add to the value
		@return			The value after <code>amount</code> was added
		@see http://msdn.microsoft.com/en-us/library/ms683614(v=vs.85).aspx
		@see http://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man3/OSAtomicAdd32Barrier.3.html
		@see http://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Atomic-Builtins.html
	*/
	inline AtomicInteger::NativeInt AtomicInteger::_add(NativeInt amount) {;
		#if WinInterlock
			return static_cast<NativeInt>(InterlockedAdd(&_number, amount));
		#elif GCCBuiltInAtomic
			return __sync_add_and_fetch(&_number, amount);
		#elif MacAtomic
			return OSAtomicAdd32Barrier(amount, &_number);
		#endif
	}
}

#undef AtomicAddFunction

#endif // __AtomicInteger_h__
