#ifndef __EString_h__
#define __EString_h__

#include <string>

#ifndef trace_block
	#define trace_block
#endif

/** String with similar interface as std::string, but doesn't have it's own storage.
	References sections of a string storage.
	This is particularly useful when you have a large string buffer (contents of a file)
		and you are going to be, for instance, tokenizing the contents. No new allocations
		occur, everything is stack based.
	@todo Templatize on character type. Allow for UTF-8.
*/
class EString {
	public:
		/// Constant represents the end of the string or not found.
		static const size_t	npos= static_cast<size_t>(-1);
		/// Default constructor, plus initialization constructor.
		EString(const void *buffer= NULL, size_t bufferSize= 0);
		/// Construct from a std::string
		EString(const std::string &str, size_t offset= 0, size_t subSize= npos);
		/// Copy constructor, plus options.
		EString(const EString &other, size_t offset= 0, size_t subSize= npos);
		/// Destructor.
		~EString();
		/// Assignment.
		EString &operator=(const EString &other);
		/// Determines if this string actually has contents.
		operator bool() const;
		/// Bounds checked character access.
		const char &operator[](size_t index) const;
		/// Binary patterns are the same.
		bool operator==(const EString &str) const;
		/// Binary patterns are not the same.
		bool operator!=(const EString &str) const;
		/// Binary pattern is less than another.
		bool operator<(const EString &str) const;
		/// Binary pattern is greater than another.
		bool operator>(const EString &str) const;
		/// Binary pattern is less than or equal to another.
		bool operator<=(const EString &str) const;
		/// Binary pattern is greater than or equal to another.
		bool operator>=(const EString &str) const;
		/// Unsafe character access, no bounds checking.
		const char &getRaw(size_t index) const;
		/// Get a std::string representation.
		std::string string() const;
		/// Get the raw pointer to the beginning of this string.
		const char * const data() const;
		/// Determines if two strings point to the same memory location.
		bool sameAddress(const EString &str) const;
		/// Gets the length of the string.
		size_t size() const;
		/// Gets the length of the string.
		size_t length() const;
		/// Does this string have contents.
		bool valid() const;
		/// Compare two strings.
		int compare(const EString &str) const;
		/// Removes count bytes from the beginning of the string.
		EString &trimFromStart(size_t count);
		/// Removes count bytes from the end of the string.
		EString &trimFromEnd(size_t count);
		/// Copies (possibly a substring of) another string.
		EString &assign(const EString &str, size_t offset, size_t subSize= npos);
		/// Returns a substring.
		EString substring(size_t offset, size_t subSize= npos) const;
		/// Finds the given character's position.
		size_t find(char character, size_t offset= 0) const;
		/// Finds the given substring in this string.
		size_t find(const EString &str, size_t offset= 0) const;
	private:
		const void *_buffer;	///< The start of this string in the buffer.
		size_t		_size;		///< The number of bytes of this string in the buffer.

		/// Raw search for the given character starting at the given offset.
		size_t _find(char character, size_t offset) const;
		/// Calculates the address to use for a substring.
		template<class String>
		static const void *_substringAddress(const String &str, size_t offset, size_t subSize);
		/// Calculates the size of a substring.
		template<class String>
		static size_t _substringSize(const String &str, size_t offset, size_t subSize);
};

/** @name Constructors and Destructors
*/ // @{
/**
	@param buffer		The buffer to reference, or NULL for empty string.
	@param bufferSize	The number of bytes in the referenced <code>buffer</code> or 0 for empty string.
*/
inline EString::EString(const void *buffer, size_t bufferSize)
	:_buffer(bufferSize <= 0 ? NULL : buffer), _size(NULL == buffer ? 0 : bufferSize) {trace_block
}
/**
	@note <code>str</code> must not go out of scope, or be destructed until <code>this</code>
		and all derived strings are cleared out.
	@param str		The string to get a substring of.
	@param offset	The offset in <code>other</code>. Defaults to the beginning.
	@param subSize	The number of bytes to references. Defaults to npos, or the rest of the string.
*/
EString::EString(const std::string &str, size_t offset, size_t subSize)
	:_buffer(_substringAddress(str, offset, subSize)),
		_size(_substringSize(str, offset, subSize)) {trace_block
}
/**
	@param other	The other string to get a substring of.
	@param offset	The offset in <code>other</code>. Defaults to the beginning.
	@param subSize	The number of bytes to references. Defaults to npos, or the rest of the string.
*/
inline EString::EString(const EString &other, size_t offset, size_t subSize)
	:_buffer(_substringAddress(other, offset, subSize)),
		_size(_substringSize(other, offset, subSize)) {trace_block
}
/** Does nothing. */
inline EString::~EString() {trace_block}
// @}
/** @name operators
	Helper functions to make it easier to use in C++.
*/ //@{
/**
	@param other	The string to copy.
	@return			Reference to <code>this</code>.
*/
inline EString &EString::operator=(const EString &other) {trace_block
	_buffer= other._buffer;
	_size= other._size;
	return *this;
}
/** See valid().
	@return	<code>true</code> if this string is not empty.
*/
inline EString::operator bool() const {trace_block
	return valid();
}
/**
	@param index	The index into the string.
	@return			A reference to the character at the given index, or a references to a null
						character if index is out of bounds.
*/
inline const char &EString::operator[](size_t index) const {trace_block
	static const char	kNull= '\0';

	if( (index >= _size) || (NULL == _buffer) || (_size <= 0) ) {trace_block
		return kNull;
	}
	return getRaw(index);
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if the strings are identical.
*/
inline bool EString::operator==(const EString &str) const {trace_block
	if(_size != str._size) {trace_block
		return false;
	}
	if(_buffer == str._buffer) {trace_block
		return true;
	}
	return compare(str) == 0;
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if the strings are not identical.
*/
inline bool EString::operator!=(const EString &str) const {trace_block
	if(_size != str._size) {trace_block
		return true;
	}
	if(_buffer == str._buffer) {trace_block
		return false;
	}
	return compare(str) != 0;
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is less.
*/
inline bool EString::operator<(const EString &str) const {trace_block
	return compare(str) < 0;
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is greater.
*/
inline bool EString::operator>(const EString &str) const {trace_block
	return compare(str) > 0;
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is less or identical.
*/
inline bool EString::operator<=(const EString &str) const {trace_block
	return compare(str) <= 0;
}
/** See compare(const EString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is greater or identical.
*/
inline bool EString::operator>=(const EString &str) const {trace_block
	return compare(str) >= 0;
}
// @}
/** @name Accessing Data
	Methods to get bytes out of the string.
*/ // @{
/** There is not bounds checking, so this should be used with caution.
	@param index	The index into the string.
	@return			A reference to the character at the given index.
*/
inline const char &EString::getRaw(size_t index) const {trace_block
	return reinterpret_cast<const char*>(_buffer)[index];
}
/** Copies the bytes into a std::string.
	@return	the std::string with the byte contents of <code>this</code>.
*/
inline std::string EString::string() const {trace_block
	return std::string(&(*this)[0], _size);
}
/** Gets the raw pointer to the beginning of <code>this</code>.
	@return	The address of the buffer, or NULL if the string is empty.
*/
inline const char * const EString::data() const {trace_block
	if(_size > 0) {trace_block
		return reinterpret_cast<const char * const>(_buffer);
	}
	return NULL;
}
/**
	@param str		The string to copy.
	@param offset	The offset in <code>str</code> to start copying.
	@param subSize	The number of bytes to copy.
	@return			Reference to <code>this</code>.
	@note When we say "copy" there is not copy that is actually done.
*/
inline EString &EString::assign(const EString &str, size_t offset, size_t subSize) {trace_block
	if(npos == subSize) {trace_block
		subSize= str._size - offset;
	}
	if( (offset >= str._size) || (subSize <= 0) || (subSize + offset > str._size) ) {trace_block
		_buffer= NULL;
		_size= 0;
	} else {trace_block
		_buffer= &str[offset];
		_size= subSize;
	}
	return *this;
}
/**
	@param offset	The offset of the substring.
	@param subSize	The number of bytes in the substring.
	@return			The substring.
*/
inline EString EString::substring(size_t offset, size_t subSize) const {trace_block
	if(npos == subSize) {trace_block
		subSize= _size - offset;
	}
	if( (NULL == _buffer) || (0 == _size)
		|| (offset > _size)
		|| (subSize <= 0) || (offset + subSize > _size) ) {trace_block
		return EString();
	}
	return EString(&reinterpret_cast<const char*>(_buffer)[offset], subSize);
}
// @}
/** @name Getting Information
	Methods to get information from the string.
*/ // @{
/** Determines if two string reference the same address in memory.
	@param str	Other string.
	@return		<code>true</code> if they both references the same exact address
					for the first byte of the string.
*/
inline bool EString::sameAddress(const EString &str) const {trace_block
	return _buffer == str._buffer;
}
/** See size(). For coding ease, depending on what language/paradigm you come from.
	@return	The number of bytes in <code>this</code>.
*/
inline size_t EString::size() const {trace_block
	return _size;
}
/** See length(). For coding ease, depending on what language/paradigm you come from.
	@return	The number of bytes in <code>this</code>.
*/
inline size_t EString::length() const {trace_block
	return _size;
}
/**
	@return	<code>true</code> if there is a buffer and we are referencing one or more bytes of it.
*/
inline bool EString::valid() const {trace_block
	return (NULL != _buffer) && (_size > 0);
}
/** Compares the bytes, starting at index 0. The bytes are compared as unsigned characters.
	The first non-matching byte index is used for the comparison.
	@return <ul>
				<li><b>-1</b> if <code>this</code> is less than <code>str</code>
				<li><b>0</b> if <code>this</code> identical to <code>str</code>
				<li><b>1</b> if <code>this</code> is greater than <code>str</code>
			</ul>
*/
// @}
inline int EString::compare(const EString &str) const {trace_block
	const unsigned char	*s1= reinterpret_cast<const unsigned char*>(_buffer);
	const unsigned char	*s2= reinterpret_cast<const unsigned char*>(str._buffer);

	for(size_t index= 0; (index < _size) && (index < str._size); ++index) {trace_block
		if(s1[index] < s2[index]) {trace_block
			return -1;
		} else if(s1[index] > s2[index]) {trace_block
			return 1;
		}
	}
	if(_size < str._size) {trace_block
		return -1;
	} else if(_size > str._size) {trace_block
		return 1;
	}
	return 0;
}
/** If everything is removed, this string becomes invalid. See valid(). See operator bool().
	<code>this</code> is modified in place.
	@param count	The number of bytes to remove from the beginning of the string.
	@return			Reference to <code>this</code>.
*/
inline EString &EString::trimFromStart(size_t count) {trace_block
	if(count >= _size) {trace_block
		_buffer= NULL;
		_size= 0;
	} else {trace_block
		_buffer= &(*this)[count];
		_size-= count;
	}
	return *this;
}
/** If everything is removed, this string becomes invalid. See valid(). See operator bool().
	<code>this</code> is modified in place.
	@param count	The number of bytes to remove from the end of the string.
	@return			Reference to <code>this</code>.
*/
inline EString &EString::trimFromEnd(size_t count) {trace_block
	if(count > _size) {trace_block
		_buffer= NULL;
		_size= 0;
	} else {trace_block
		_size-= count;
	}
	return *this;
}
/** Short circuits the search if <code>this</code> is not valid or the offset is past the end.
	@param character	The byte to search for.
	@param offset		The position to start looking for <code>character</code>. Default is index 0.
*/
inline size_t EString::find(char character, size_t offset) const {trace_block
	if( (NULL == _buffer) || (0 == _size) || (offset >= _size) ) {trace_block
		return npos;
	}
	return _find(character, offset);
}
/** Short circuits the search if <code>this</code> or <code>str</code> is not valid,
		or <code>str</code> could not fit in the space left after offset.
	@param str			The substring to search for.
	@param offset		The position to start looking for <code>str</code>. Default is index 0.
*/
inline size_t EString::find(const EString &str, size_t offset) const {trace_block
	const size_t	kNpos= npos;

	if( (NULL == _buffer) || (0 == _size)
		|| (NULL == str._buffer) || (0 == str._size)
		|| (offset + str._size >= _size) ) {trace_block
		return kNpos;
	}
	while(kNpos != offset) {trace_block
		offset= _find(str[0], offset);
		if(kNpos != offset) {trace_block
			if(substring(offset, str._size) == str) {trace_block
				return offset;
			}
			++offset;
		}
	}
	return kNpos;
}
/** Core character search. _buffer and _size must be valid before calling.
	@param character	The byte to search for.
	@param offset		The position to start looking for <code>character</code>. Default is index 0.
*/
inline size_t EString::_find(char character, size_t offset) const {trace_block
	const unsigned char	unsignedCharacter= static_cast<unsigned char>(character);
	const char			*start= reinterpret_cast<const char*>(_buffer);
	const char			*found= reinterpret_cast<const char*>(memchr(&start[offset],
																		unsignedCharacter,
																		_size - offset
																	)
															);

	if(NULL == found) {trace_block
		return npos;
	}
	return found - start;
}
/** Helper function for constructor's ctor list.
	@param str		The string to get the address of.
	@param offset	The offset in <code>str</code>.
	@param subSize	The size of the substring.
	@return			NULL if <code>str</code> is not valid,
						or <code>offset + subSize</code> would exceed <code>str</code>
					otherwise <code>str._buffer + offset</code> is returned.
*/
template<class String>
const void *EString::_substringAddress(const String &str, size_t offset, size_t subSize) {trace_block
	if( (NULL == str.data()) || (str.size() <= 0) || (offset >= str.size()) ) {trace_block
		return NULL;
	}
	if(npos == subSize) {trace_block
		subSize= str.size() - offset;
	}
	if( (subSize <= 0) || (offset + subSize > str.size()) ) {trace_block
		return NULL;
	}
	return &str.data()[offset];
}
/** Helper function for constructor's ctor list.
	@param str		The string to evaluate.
	@param offset	The offset in <code>str</code>.
	@param subSize	The size of the substring.
	@return			0 if <code>str</code> is not valid,
						or <code>offset + subSize</code> would exceed <code>str</code>
					otherwise <code>subSize</code> is returned.
*/
template<class String>
size_t EString::_substringSize(const String &str, size_t offset, size_t subSize) {trace_block
	if( (NULL == str.data()) || (str.size() <= 0) || (offset >= str.size()) ) {trace_block
		return 0;
	}
	if(npos == subSize) {trace_block
		subSize= str.size() - offset;
	}
	if( (subSize <= 0) || (offset + subSize > str.size()) ) {trace_block
		return 0;
	}
	return subSize;
}

#endif // __EString_h__
