#ifndef __TypeSafeEnum_h__
#define __TypeSafeEnum_h__

#include <cstring>
#include <exception>
#include <sstream>
#include <string>

#ifdef __tracer_h__
	#define TracePoint	passThroughLog(0, "TracePoint", __FILE__, __func__, __LINE__)
	#define TracePointValue(value)	passThroughLog(value, #value, __FILE__, __func__, __LINE__)
#else
	#define TracePoint	
	#define TracePointValue(value)
#endif

class BitOutOfRangeException : public std::exception {
	public:
		BitOutOfRangeException(int bit, int max) throw();
		BitOutOfRangeException(const BitOutOfRangeException &other);
		BitOutOfRangeException &operator=(const BitOutOfRangeException &other);
		virtual ~BitOutOfRangeException() throw();
		virtual const char* what() const throw();
	private:
		std::string	_message;
		static std::string _numbersToMessage(int bit, int max);
};

template<typename EnumType, EnumType lastEnumValue, typename WordType= unsigned char>
class TypeSafeEnum {
	public:
		TypeSafeEnum();
		TypeSafeEnum(const EnumType &bit);
		TypeSafeEnum(const TypeSafeEnum &other);
		~TypeSafeEnum();
		TypeSafeEnum &operator=(const TypeSafeEnum &other);
		bool operator==(const EnumType &other) const;
		bool operator!=(const EnumType &other) const;
		bool operator<(const EnumType &other) const;
		bool operator<=(const EnumType &other) const;
		bool operator>(const EnumType &other) const;
		bool operator>=(const EnumType &other) const;
		bool operator==(const TypeSafeEnum &other) const;
		bool operator!=(const TypeSafeEnum &other) const;
		bool operator<(const TypeSafeEnum &other) const;
		bool operator<=(const TypeSafeEnum &other) const;
		bool operator>(const TypeSafeEnum &other) const;
		bool operator>=(const TypeSafeEnum &other) const;
		TypeSafeEnum operator&(const EnumType &bit) const;
		TypeSafeEnum operator|(const EnumType &bit) const;
		TypeSafeEnum operator^(const EnumType &bit) const;
		TypeSafeEnum operator&(const TypeSafeEnum &bitField) const;
		TypeSafeEnum operator|(const TypeSafeEnum &bitField) const;
		TypeSafeEnum operator^(const TypeSafeEnum &bitField) const;
		TypeSafeEnum operator~() const;
		TypeSafeEnum &operator&=(const EnumType &bit);
		TypeSafeEnum &operator|=(const EnumType &bit);
		TypeSafeEnum &operator^=(const EnumType &bit);
		TypeSafeEnum &operator&=(const TypeSafeEnum &bitField);
		TypeSafeEnum &operator|=(const TypeSafeEnum &bitField);
		TypeSafeEnum &operator^=(const TypeSafeEnum &bitField);
		bool empty() const;
		int bits() const;
		template<typename Integer>
		Integer max() const;
		template<typename Integer>
		Integer get() const;
		template<typename Integer>
		Integer get(int startBit, int bitCount) const;
		template<typename Integer>
		TypeSafeEnum &set(const Integer &number);
		bool isSet(EnumType bit) const;
		TypeSafeEnum &set(EnumType bit);
		TypeSafeEnum &set(EnumType bit, bool value);
		TypeSafeEnum &clear(EnumType bit);
		TypeSafeEnum &clear();
		TypeSafeEnum &invert();
		friend bool operator==(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other == primary;
		}
		friend bool operator!=(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other != primary;
		}
		friend bool operator<(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other > primary;
		}
		friend bool operator<=(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other >= primary;
		}
		friend bool operator>(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other < primary;
		}
		friend bool operator>=(const EnumType &primary, const TypeSafeEnum &other) {TracePoint;
			return other <= primary;
		}
		friend TypeSafeEnum operator&(const EnumType &primary, const TypeSafeEnum &bitField) {TracePoint;
			return bitField & primary;
		}
		friend TypeSafeEnum operator|(const EnumType &primary, const TypeSafeEnum &bitField) {TracePoint;
			return bitField | primary;
		}
		friend TypeSafeEnum operator^(const EnumType &primary, const TypeSafeEnum &bitField) {TracePoint;
			return bitField ^ primary;
		}
	private:
		WordType	_bits[static_cast<size_t>(lastEnumValue) / sizeof(WordType) / 8 + 1];
		static int _translateLocation(EnumType value, WordType &mask);
};

inline BitOutOfRangeException::BitOutOfRangeException(int bit, int max) throw()
	:std::exception(), _message(_numbersToMessage(bit, max)) {TracePoint;}
inline BitOutOfRangeException::BitOutOfRangeException(const BitOutOfRangeException &other)
	:std::exception(other), _message(other._message) {TracePoint; /* Not Tested */}
inline BitOutOfRangeException &BitOutOfRangeException::operator=(const BitOutOfRangeException &other) {TracePoint; /* Not Tested */
	_message= other._message;
	return *this;
}
inline BitOutOfRangeException::~BitOutOfRangeException() throw() {TracePoint;}
inline const char* BitOutOfRangeException::what() const throw() {TracePoint; /* Not Tested */
	return _message.c_str();
}
std::string BitOutOfRangeException::_numbersToMessage(int bit, int max) {TracePoint;
	std::ostringstream stream;
	
	stream << "Bit Index out of range: Bit= " << bit << " Max=" << max;
	return stream.str();
}

template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum() 
	:_bits() {TracePoint;
	std::memset(&_bits, 0, sizeof(_bits));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum(const EnumType &bit)
	:_bits() {TracePoint;
	std::memset(&_bits, 0, sizeof(_bits));
	set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum(const TypeSafeEnum &other)
	:_bits() {TracePoint;
	*this= other;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::~TypeSafeEnum() {TracePoint;}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator=(const TypeSafeEnum &other) {TracePoint;
	if(this != &other) {TracePoint;
		for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
			_bits[word]= other._bits[word];
		}
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator==(const EnumType &other) const {TracePoint;
	return *this == TypeSafeEnum(other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator!=(const EnumType &other) const {TracePoint;
	return !(*this == TypeSafeEnum(other));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<(const EnumType &other) const {TracePoint;
	return *this < TypeSafeEnum(other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<=(const EnumType &other) const {TracePoint;
	return !(TypeSafeEnum(other) < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>(const EnumType &other) const {TracePoint;
	return TypeSafeEnum(other) < *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>=(const EnumType &other) const {TracePoint;
	return !(*this < TypeSafeEnum(other));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator==(const TypeSafeEnum &other) const {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		if(_bits[word] != other._bits[word]) {TracePoint;
			return false;
		}
	}
	return true;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator!=(const TypeSafeEnum &other) const {TracePoint;
	return !(*this == other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<(const TypeSafeEnum &other) const {TracePoint;
	for(size_t word= sizeof(_bits)/sizeof(_bits[0]); word > 0; --word) {TracePointValue(word);
		if(_bits[word - 1] > other._bits[word - 1]) {TracePoint;
			return false;
		}
	}
	return _bits[0] < other._bits[0];
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<=(const TypeSafeEnum &other) const {TracePoint;
	return !(other < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>(const TypeSafeEnum &other) const {TracePoint;
	return (other < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>=(const TypeSafeEnum &other) const {TracePoint;
	return !(*this < other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&(const EnumType &bit) const {TracePoint;
	if(isSet(bit)) {TracePoint;
		return TypeSafeEnum(bit);
	}
	return TypeSafeEnum();
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|(const EnumType &bit) const {TracePoint;
	return TypeSafeEnum(*this).set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^(const EnumType &bit) const {TracePoint;
	if(isSet(bit)) {TracePoint;
		return TypeSafeEnum(*this).clear(bit);
	}
	return TypeSafeEnum(*this).set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&(const TypeSafeEnum &bitField) const {TracePoint;
	return TypeSafeEnum(*this)&=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|(const TypeSafeEnum &bitField) const {TracePoint;
	return TypeSafeEnum(*this)|=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^(const TypeSafeEnum &bitField) const {TracePoint;
	return TypeSafeEnum(*this)^=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator~() const {TracePoint;
	return TypeSafeEnum(*this).invert();
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&=(const EnumType &bit) {TracePoint;
	const bool	wasSet= isSet(bit);
	clear();
	if(wasSet) {TracePoint;
		set(bit);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|=(const EnumType &bit) {TracePoint;
	set(bit);
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^=(const EnumType &bit) {TracePoint;
	if(isSet(bit)) {TracePoint;
		clear(bit);
	} else {TracePoint;
		set(bit);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&=(const TypeSafeEnum &bitField) {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		_bits[word]&= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|=(const TypeSafeEnum &bitField) {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		_bits[word]|= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^=(const TypeSafeEnum &bitField) {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		_bits[word]^= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::empty() const {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		if(_bits[word] != 0) {TracePoint;
			return false;
		}
	}
	return true;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
int TypeSafeEnum<EnumType, lastEnumValue, WordType>::bits() const {TracePoint;
	return static_cast<int>(lastEnumValue) + 1;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::max() const {TracePoint;
	const Integer	one(1);
	
	if(static_cast<int>(sizeof(Integer)) * 8 < bits()) {TracePoint;
		throw BitOutOfRangeException(bits(), sizeof(Integer) * 8);
	}
	Integer	result(0);
	for(size_t bit= 0; bit <= lastEnumValue; ++bit) {TracePointValue(bit);
		result<<= 1;
		result|= one;
	}
	return result;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::get() const {TracePoint;
	if(static_cast<int>(lastEnumValue) >= sizeof(Integer)*8) {TracePoint;
		throw BitOutOfRangeException(static_cast<int>(lastEnumValue), sizeof(Integer)*8 - 1);
	}
	return get<Integer>(0, sizeof(Integer) * 8);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::get(int startBit, int bitCount) const {TracePoint;
	const Integer	one(1);
	Integer			result(0);
	size_t			bit= startBit + bitCount - 1;
	
	if(static_cast<size_t>(bitCount) > sizeof(Integer) * 8) {TracePoint;
		throw BitOutOfRangeException(bitCount, sizeof(Integer) * 8);
	}
	while(static_cast<int>(bit) >= startBit) {TracePoint;
		result<<= 1;
		if(isSet(static_cast<EnumType>(bit))) {TracePoint;
			result|= one;
		}
		--bit;
		--bitCount;
	}
	return result;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(const Integer &number) {TracePoint;
	const Integer	one(1);
	Integer			value(number);
	
	clear();
	for(size_t bit= 0; bit <= static_cast<int>(lastEnumValue); ++bit) {TracePointValue(bit);
		if( (value & one) == one ) {TracePoint;
			set(static_cast<EnumType>(bit));
		}
		value>>= 1;
	}
	if(value > 0) {TracePoint;
		int	bit= static_cast<int>(lastEnumValue);
		
		while( (value & one) != one ) {TracePoint;
			value>>= 1;
			bit+= 1;
		}
		throw BitOutOfRangeException(bit, lastEnumValue);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::isSet(EnumType bit) const {TracePoint;
	WordType	mask;
	
	return (_bits[_translateLocation(bit, mask)] & mask) == mask;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(EnumType bit, bool value) {TracePoint;
	WordType	mask;
	int			word;
	
	word= _translateLocation(bit, mask);
	if(value) {TracePoint;
		_bits[word]|= mask;
	} else {TracePoint;
		_bits[word]&= ~mask;
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(EnumType bit) {TracePoint;
	return set(bit, true);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::clear(EnumType bit) {TracePoint;
	return set(bit, false);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::clear() {TracePoint;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		_bits[word]= 0;
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::invert() {TracePoint;
	WordType	lastMask;
	int			lastIndex= _translateLocation(lastEnumValue, lastMask);
	
	lastMask= (lastMask << 1) - 1;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {TracePointValue(word);
		_bits[word]= ~_bits[word];
	}
	_bits[lastIndex]= lastMask & _bits[lastIndex];
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
int TypeSafeEnum<EnumType, lastEnumValue, WordType>::_translateLocation(EnumType value, WordType &mask) {TracePoint;
	int	asInt= static_cast<int>(value);
	
	mask= int(1) << (asInt % (8*sizeof(WordType)));
	return asInt / sizeof(WordType) / 8;
}

#endif // __TypeSafeEnum_h__
