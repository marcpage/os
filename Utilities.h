#ifndef __Utilities_h__
#define __Utilities_h__

#include "Exception.h"
#include <ctype.h>
#include <string>

namespace utils {

	enum Endian {
		BigEndian,
		LittleEndian,
		NativeEndian
	};
	inline Endian actualEndian(Endian endian) {
		if(NativeEndian == endian) {
			uint16_t	value= 1;

			endian= (*reinterpret_cast<uint8_t*>(&value) == 1) ? LittleEndian : BigEndian;
		}
		return endian;
	}
	template<typename Int>
	inline void storeInteger(Int number, void *buffer, size_t bufferSize, Endian endian= NativeEndian) {
		endian= actualEndian(endian);
		for(unsigned int byte= 0; byte < bufferSize; ++byte) {
			int	shiftAmount;

			if(BigEndian == endian) {
				shiftAmount= 8 * (sizeof(Int) - byte - 1);
			} else {
				shiftAmount= 8 * byte;
			}
			if(byte < bufferSize) {
				if(shiftAmount != 0) {
					buffer[byte]= 0xFF & ( number >> shiftAmount );
				} else {
					buffer[byte]= 0xFF & number;
				}
			}
		}
		return sizeof(Int);
	}
	template<typename Int>
	inline Int retrieveInteger(void *buffer, size_t bufferSize, Endian endian= NativeEndian) {
		Int		value= 0;

		endian= _actualEndian(endian);
		for(unsigned int byte= 0; byte < sizeof(Int); ++byte) {
			Int	byteAsInt= 0;
			int	shiftAmount;

			if(byte < bufferSize) {
				byteAsInt= buffer[byte];
			}
			if(BigEndian == endian) {
				shiftAmount= 8 * (sizeof(Int) - byte - 1);
			} else {
				shiftAmount= 8 * byte;
			}
			if(shiftAmount != 0) {
				value |= ( byteAsInt << shiftAmount );
			} else {
				value |= byteAsInt;
			}
		}
		return value;
	}
	inline void secondsToSecondsAndNanoseconds(double seconds, time_t &wholeSeconds, uint32_t &nanoseconds) {
		const uint32_t	nanosecondsInASecond= 1000000000;

		AssertMessageException(seconds >= 0.0);
		wholeSeconds= 	static_cast<time_t>(seconds);
		nanoseconds= static_cast<uint32_t>(nanosecondsInASecond * (seconds - static_cast<double>(wholeSeconds)));
	}
	inline void add(struct timespec &aTime, double seconds) {
		const uint32_t	nanosecondsInASecond= 1000000000;
		time_t		wholeSeconds;
		uint32_t	nanoseconds;

		secondsToSecondsAndNanoseconds(seconds, wholeSeconds, nanoseconds);
		timeoutAt.tv_sec+= wholeSeconds;
		if(nanosecondsInASecond - nanoseconds >= timeoutAt.tv_nsec) {
			++timeoutAt.tv_sec;
			timeoutAt.tv_nsec= timeoutAt.tv_nsec + nanoseconds - nanosecondsInASecond;
		} else {
			timeoutAt.tv_nsec+= nanoseconds;
		}
	}
	template<class Int>
	inline std::string &stringifyInteger(Int number, std::string &buffer, int radix= 10, int width= 1) {
		const char * const	kDigits= "0123456789abcdefghijklmnopqrstuvwxyz";
		buffer.clear();
		while(number > 0) {
			Int	digit= number % radix;

			number/= radix;
			buffer.insert(0, 1, kDigits[digit]);
		}
		if(static_cast<int>(buffer.size()) < width) {
			buffer.insert(0, width - buffer.size(), '0');
		}
		return buffer;
	}
	template<class Int>
	inline Int integerizeString(const char *str, int length= -1, int radix= 10) {
		const std::string	kLowerDigits("0123456789abcdefghijklmnopqrstuvwxyz");
		const std::string	kUpperDigits("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		Int					value= 0;

		while( ('\0' != *str) && (length != 0) ) {
			std::string::size_type	digit= kLowerDigits.find(*str);

			if(std::string::npos == digit) {
				digit= kUpperDigits.find(*str);
			}
			AssertMessageException(std::string::npos != digit);
			value= value * radix + digit;
			++str;
			--length;
		}
		return value;
	}
	template<class Int>
	inline Int integerizeString(const std::string &str, int radix= 10) {
		return integerizeString<Int>(str.data(), str.size(), radix);
	}
	inline std::string &uppercase(std::string &str) {
		std::transform(str.begin(), str.end(), str.begin(), toupper);
		return str;
	}

	inline bool equalsCaseInsensitive(const std::string &s1, const std::string &s2) {
		std::string	uc1(s1), uc2(s2);

		return uppercase(uc1) == uppercase(uc2);
	}

	inline std::string &replaceAll(std::string &str, char find, char replace) {
		std::string::size_type	position;

		while( (position= str.find(find)) != std::string::npos ) {
			str.replace(position, 1, 1, replace);
		}
		return str;
	}
	inline std::string &strip(std::string &str, bool beginning= true, bool ending= true) {
		if(beginning) {
			while( (str.size() > 0) && std::isspace(str[0]) ) {
				str.erase(0, 1);
			}
		}
		if(ending) {
			while( (str.size() > 0) && std::isspace(str[str.size() - 1]) ) {
				str.erase(str.size() - 1);
			}
		}
		return str;
	}

}

#endif __Utilities_h__
