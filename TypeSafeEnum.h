#ifndef __TypeSafeEnum_h__
#define __TypeSafeEnum_h__

/** @file TypeSafeEnum.h
	@todo use namespace
	@todo document
	@todo Rename to EnumSet
	@todo inline friends out of class
	@todo trace_bool in for loops
*/
#include <cstring>
#include <exception>
#include <sstream>
#include <string>

#ifndef trace_scope
	#define trace_scope ///< in case Trace.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) x ///< in case Trace.h is not included
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
		friend bool operator==(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other == primary;
		}
		friend bool operator!=(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other != primary;
		}
		friend bool operator<(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other > primary;
		}
		friend bool operator<=(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other >= primary;
		}
		friend bool operator>(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other < primary;
		}
		friend bool operator>=(const EnumType &primary, const TypeSafeEnum &other) {trace_scope
			return other <= primary;
		}
		friend TypeSafeEnum operator&(const EnumType &primary, const TypeSafeEnum &bitField) {trace_scope
			return bitField & primary;
		}
		friend TypeSafeEnum operator|(const EnumType &primary, const TypeSafeEnum &bitField) {trace_scope
			return bitField | primary;
		}
		friend TypeSafeEnum operator^(const EnumType &primary, const TypeSafeEnum &bitField) {trace_scope
			return bitField ^ primary;
		}
	private:
		WordType	_bits[static_cast<size_t>(lastEnumValue) / sizeof(WordType) / 8 + 1];
		static int _translateLocation(EnumType value, WordType &mask);
};

inline BitOutOfRangeException::BitOutOfRangeException(int bit, int max) throw()
	:std::exception(), _message(_numbersToMessage(bit, max)) {trace_scope}
/**
	@todo TEST!
*/
inline BitOutOfRangeException::BitOutOfRangeException(const BitOutOfRangeException &other)
	:std::exception(other), _message(other._message) {trace_scope /* Not Tested */}
/**
	@todo TEST!
*/
inline BitOutOfRangeException &BitOutOfRangeException::operator=(const BitOutOfRangeException &other) {trace_scope /* Not Tested */
	_message= other._message;
	return *this;
}
inline BitOutOfRangeException::~BitOutOfRangeException() throw() {trace_scope}
/**
	@todo TEST!
*/
inline const char* BitOutOfRangeException::what() const throw() {trace_scope /* Not Tested */
	return _message.c_str();
}
std::string BitOutOfRangeException::_numbersToMessage(int bit, int max) {trace_scope
	std::ostringstream stream;

	stream << "Bit Index out of range: Bit= " << bit << " Max=" << max;
	return stream.str();
}

template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum()
	:_bits() {trace_scope
	std::memset(&_bits, 0, sizeof(_bits));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum(const EnumType &bit)
	:_bits() {trace_scope
	std::memset(&_bits, 0, sizeof(_bits));
	set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::TypeSafeEnum(const TypeSafeEnum &other)
	:_bits() {trace_scope
	*this= other;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType>::~TypeSafeEnum() {trace_scope}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator=(const TypeSafeEnum &other) {trace_scope
	if(this != &other) {trace_scope
		for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
			_bits[word]= other._bits[word];
		}
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator==(const EnumType &other) const {trace_scope
	return *this == TypeSafeEnum(other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator!=(const EnumType &other) const {trace_scope
	return !(*this == TypeSafeEnum(other));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<(const EnumType &other) const {trace_scope
	return *this < TypeSafeEnum(other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<=(const EnumType &other) const {trace_scope
	return !(TypeSafeEnum(other) < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>(const EnumType &other) const {trace_scope
	return TypeSafeEnum(other) < *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>=(const EnumType &other) const {trace_scope
	return !(*this < TypeSafeEnum(other));
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator==(const TypeSafeEnum &other) const {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		if(_bits[word] != other._bits[word]) {trace_scope
			return false;
		}
	}
	return true;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator!=(const TypeSafeEnum &other) const {trace_scope
	return !(*this == other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<(const TypeSafeEnum &other) const {trace_scope
	for(size_t word= sizeof(_bits)/sizeof(_bits[0]); word > 0; --word) {
		if(_bits[word - 1] > other._bits[word - 1]) {trace_scope
			return false;
		}
	}
	return _bits[0] < other._bits[0];
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator<=(const TypeSafeEnum &other) const {trace_scope
	return !(other < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>(const TypeSafeEnum &other) const {trace_scope
	return (other < *this);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator>=(const TypeSafeEnum &other) const {trace_scope
	return !(*this < other);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&(const EnumType &bit) const {trace_scope
	if(isSet(bit)) {trace_scope
		return TypeSafeEnum(bit);
	}
	return TypeSafeEnum();
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|(const EnumType &bit) const {trace_scope
	return TypeSafeEnum(*this).set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^(const EnumType &bit) const {trace_scope
	if(isSet(bit)) {trace_scope
		return TypeSafeEnum(*this).clear(bit);
	}
	return TypeSafeEnum(*this).set(bit);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&(const TypeSafeEnum &bitField) const {trace_scope
	return TypeSafeEnum(*this)&=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|(const TypeSafeEnum &bitField) const {trace_scope
	return TypeSafeEnum(*this)|=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^(const TypeSafeEnum &bitField) const {trace_scope
	return TypeSafeEnum(*this)^=bitField;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator~() const {trace_scope
	return TypeSafeEnum(*this).invert();
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&=(const EnumType &bit) {trace_scope
	const bool	wasSet= isSet(bit);
	clear();
	if(wasSet) {trace_scope
		set(bit);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|=(const EnumType &bit) {trace_scope
	set(bit);
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^=(const EnumType &bit) {trace_scope
	if(isSet(bit)) {trace_scope
		clear(bit);
	} else {trace_scope
		set(bit);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator&=(const TypeSafeEnum &bitField) {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		_bits[word]&= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator|=(const TypeSafeEnum &bitField) {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		_bits[word]|= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::operator^=(const TypeSafeEnum &bitField) {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		_bits[word]^= bitField._bits[word];
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::empty() const {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		if(_bits[word] != 0) {trace_scope
			return false;
		}
	}
	return true;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
int TypeSafeEnum<EnumType, lastEnumValue, WordType>::bits() const {trace_scope
	return static_cast<int>(lastEnumValue) + 1;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::max() const {trace_scope
	const Integer	one(1);

	if(static_cast<int>(sizeof(Integer)) * 8 < bits()) {trace_scope
		throw BitOutOfRangeException(bits(), sizeof(Integer) * 8);
	}
	Integer	result(0);
	for(size_t bit= 0; bit <= lastEnumValue; ++bit) {
		result<<= 1;
		result|= one;
	}
	return result;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::get() const {trace_scope
	if(static_cast<int>(lastEnumValue) >= sizeof(Integer)*8) {trace_scope
		throw BitOutOfRangeException(static_cast<int>(lastEnumValue), sizeof(Integer)*8 - 1);
	}
	return get<Integer>(0, sizeof(Integer) * 8);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
Integer TypeSafeEnum<EnumType, lastEnumValue, WordType>::get(int startBit, int bitCount) const {trace_scope
	const Integer	one(1);
	Integer			result(0);
	size_t			bit= startBit + bitCount - 1;

	if(static_cast<size_t>(bitCount) > sizeof(Integer) * 8) {trace_scope
		throw BitOutOfRangeException(bitCount, sizeof(Integer) * 8);
	}
	while(static_cast<int>(bit) >= startBit) {trace_scope
		result<<= 1;
		if(isSet(static_cast<EnumType>(bit))) {trace_scope
			result|= one;
		}
		--bit;
		--bitCount;
	}
	return result;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
template<typename Integer>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(const Integer &number) {trace_scope
	const Integer	one(1);
	Integer			value(number);

	clear();
	for(size_t bit= 0; bit <= static_cast<int>(lastEnumValue); ++bit) {
		if( (value & one) == one ) {trace_scope
			set(static_cast<EnumType>(bit));
		}
		value>>= 1;
	}
	if(value > 0) {trace_scope
		int	bit= static_cast<int>(lastEnumValue);

		while( (value & one) != one ) {trace_scope
			value>>= 1;
			bit+= 1;
		}
		throw BitOutOfRangeException(bit, lastEnumValue);
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
bool TypeSafeEnum<EnumType, lastEnumValue, WordType>::isSet(EnumType bit) const {trace_scope
	if(bit >= lastEnumValue) {
		return false;
	}
	WordType	mask;
	int			location= _translateLocation(bit, mask);
	const bool	result= (_bits[location] & mask) == mask;

	return result;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(EnumType bit, bool value) {trace_scope
	WordType	mask;
	int			word;

	word= _translateLocation(bit, mask);
	if(value) {trace_scope
		_bits[word]|= mask;
	} else {trace_scope
		_bits[word]&= ~mask;
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::set(EnumType bit) {trace_scope
	return set(bit, true);
}
/**
	@todo TEST!
*/
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::clear(EnumType bit) {trace_scope
	return set(bit, false);
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::clear() {trace_scope
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		_bits[word]= 0;
	}
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
TypeSafeEnum<EnumType, lastEnumValue, WordType> &TypeSafeEnum<EnumType, lastEnumValue, WordType>::invert() {trace_scope
	WordType	lastMask;
	int			lastIndex= _translateLocation(lastEnumValue, lastMask);

	lastMask= (lastMask << 1) - 1;
	for(size_t word= 0; word < sizeof(_bits)/sizeof(_bits[0]); ++word) {
		_bits[word]= ~_bits[word];
	}
	_bits[lastIndex]= lastMask & _bits[lastIndex];
	return *this;
}
template<typename EnumType, EnumType lastEnumValue, typename WordType>
int TypeSafeEnum<EnumType, lastEnumValue, WordType>::_translateLocation(EnumType value, WordType &mask) {trace_scope
	const int	asInt= static_cast<int>(value);
	const int	location= asInt / sizeof(WordType) / 8;

	mask= int(1) << (asInt % (8*sizeof(WordType)));
	return location;
}

#endif // __TypeSafeEnum_h__
