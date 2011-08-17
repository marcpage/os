#ifndef __CPlusPlusUtility_h__
#define __CPlusPlusUtility_h__

#include "Exception.h"

namespace cppu {

	template<typename Integer>
	std::string dec(Integer value, int padding= 0, char pad= ' ') {
		std::stringstream	ss;

		ss.width(padding);
		ss.fill(pad);
		ss << value;
		return ss.str();
	}
	template<>
	std::string dec(int8_t value, int padding, char pad) {
		std::string	suffix;

		if(isprint(value)) {
			suffix.assign(" (").append(dec(value)).append(")");
		}
		return dec(static_cast<int64_t>(value), padding, pad)+suffix;
	}
	template<>
	std::string dec(uint8_t value, int padding, char pad) {
		return dec(static_cast<int8_t>(value), padding, pad);
	}
	template<typename Integer>
	std::string hex(Integer value, int padding= 0, char pad= ' ') {
		std::stringstream	ss;

		ss.width(padding);
		ss.fill(pad);
		ss << "0x" << std::hex << value;
		return ss.str();
	}
	template<>
	std::string hex(int8_t value, int padding, char pad) {
		return hex(static_cast<int64_t>(value), padding, pad);
	}
	template<>
	std::string hex(uint8_t value, int padding, char pad) {
		return hex(static_cast<int64_t>(value), padding, pad);
	}
	template<typename Integer>
	std::string oct(Integer value, int padding= 0, char pad= ' ') {
		std::stringstream	ss;

		ss.width(padding);
		ss.fill(pad);
		ss << "0" << std::oct << value;
		return ss.str();
	}
	template<>
	std::string oct(int8_t value, int padding, char pad) {
		return oct(static_cast<int64_t>(value), padding, pad);
	}
	template<>
	std::string oct(uint8_t value, int padding, char pad) {
		return oct(static_cast<int64_t>(value), padding, pad);
	}
	template<typename Integer>
	std::string si(Integer value, int padding= 0, char pad= ' ') {
		if(padding > 3) {
			padding-= 2;
		}
		if(value < 1024) {
			return dec(value, padding, pad) + "Bi";
		}
		value/= 1024;
		if(value < 1024) {
			return dec(value, padding, pad) + "Ki";
		}
		value/= 1024;
		if(value < 1024) {
			return dec(value, padding, pad) + "Mi";
		}
		value/= 1024;
		if(value < 1024) {
			return dec(value, padding, pad) + "Gi";
		}
		value/= 1024;
		return dec(value, padding, pad) + "Ti";
	}
	template<>
	std::string si(uint8_t value, int padding, char pad) {
		return si(static_cast<uint64_t>(value), padding, pad);
	}
	template<>
	std::string si(int8_t value, int padding, char pad) {
		return si(static_cast<int64_t>(value), padding, pad);
	}
	template<typename Number>
	Number dec(const std::string &asString) {
		std::istringstream	i(asString);
		Number				x;
		bool				ableToConvertStringToNumeric= (i >> x);

		AssertMessageException(ableToConvertStringToNumeric);
		return x;
	}

}


#endif //__CPlusPlusUtility_h__

