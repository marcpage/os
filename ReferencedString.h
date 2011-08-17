#ifndef __ReferencedString_h__
#define __ReferencedString_h__

/** @file ReferencedString.h
	@todo trace_bool in for loops
*/
#include <string>

#ifndef trace_scope
	#define trace_scope ///< in case Trace.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) x ///< in case Trace.h is not included
#endif

/** String with similar interface as std::string, but doesn't have it's own storage.
	References sections of a string storage.
	This is particularly useful when you have a large string buffer (contents of a file)
		and you are going to be, for instance, tokenizing the contents. No new allocations
		occur, everything is stack based.
	@todo Templatize on character type. Allow for UTF-8.
*/
class ReferencedString {
	public:
		/// Constant represents the end of the string or not found.
		static const size_t	npos= static_cast<size_t>(-1);
		/// Default constructor.
		ReferencedString();
		/// Reference a C string.
		ReferencedString(const char * const buffer);
		/// Reference an arbitrary buffer.
		ReferencedString(const void *buffer, size_t bufferSize);
		/// Construct from a std::string
		ReferencedString(const std::string &str, size_t offset= 0, size_t subSize= npos);
		/// Copy constructor, plus options.
		ReferencedString(const ReferencedString &other, size_t offset= 0, size_t subSize= npos);
		/// Destructor.
		~ReferencedString();
		/// Assignment.
		ReferencedString &operator=(const ReferencedString &other);
		/// Determines if this string actually has contents.
		operator bool() const;
		/// Bounds checked character access.
		const char &operator[](size_t index) const;
		/// Binary patterns are the same.
		bool operator==(const ReferencedString &str) const;
		/// Binary patterns are not the same.
		bool operator!=(const ReferencedString &str) const;
		/// Binary pattern is less than another.
		bool operator<(const ReferencedString &str) const;
		/// Binary pattern is greater than another.
		bool operator>(const ReferencedString &str) const;
		/// Binary pattern is less than or equal to another.
		bool operator<=(const ReferencedString &str) const;
		/// Binary pattern is greater than or equal to another.
		bool operator>=(const ReferencedString &str) const;
		/// Unsafe character access, no bounds checking.
		const char &getRaw(size_t index) const;
		/// Get a std::string representation.
		std::string string() const;
		/// Get the raw pointer to the beginning of this string.
		const char * const data() const;
		/// Determines if two strings point to the same memory location.
		bool sameAddress(const ReferencedString &str) const;
		/// Gets the length of the string.
		size_t size() const;
		/// Gets the length of the string.
		size_t length() const;
		/// Does this string have contents.
		bool valid() const;
		/// Compare two strings.
		int compare(const ReferencedString &str) const;
		/// Removes count bytes from the beginning of the string.
		ReferencedString &trimFromStart(size_t count);
		/// Removes count bytes from the end of the string.
		ReferencedString &trimFromEnd(size_t count);
		/// Copies (possibly a substring of) another string.
		ReferencedString &assign(const ReferencedString &str, size_t offset= 0, size_t subSize= npos);
		/// Returns a substring.
		ReferencedString substring(size_t offset, size_t subSize= npos) const;
		/// Finds the given character's position.
		size_t find(char character, size_t offset= 0) const;
		/// Finds the given substring in this string.
		size_t find(const ReferencedString &str, size_t offset= 0) const;
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
/** An invalid, empty string.
*/
inline ReferencedString::ReferencedString()
	:_buffer(NULL), _size(0) {trace_scope
}
/**
	@param buffer	The null-terminated C-string to reference.
	@note <code>buffer</code> <b>must</b> be a valid pointer for the life of
				this and all derived <code>ReferenceString</code>s.
*/
inline ReferencedString::ReferencedString(const char * const buffer)
	:_buffer(buffer), _size(trace_bool(NULL == buffer) ? 0 : strlen(buffer)) {trace_scope
}
/**
	@param buffer		The buffer to reference, or NULL for empty string.
	@param bufferSize	The number of bytes in the referenced <code>buffer</code> or 0 for empty string.
	@note <code>buffer</code> <b>must</b> be a valid pointer for the life of
				this and all derived <code>ReferenceString</code>s.
*/
inline ReferencedString::ReferencedString(const void *buffer, size_t bufferSize)
	:_buffer(trace_bool(bufferSize <= 0) ? NULL : buffer), _size(trace_bool(NULL == buffer) ? 0 : bufferSize) {trace_scope
}
/**
	@note <code>str</code> must not go out of scope, or be destructed until <code>this</code>
		and all derived strings are cleared out.
	@param str		The string to get a substring of.
	@param offset	The offset in <code>other</code>. Defaults to the beginning.
	@param subSize	The number of bytes to references. Defaults to npos, or the rest of the string.
	@note <code>str</code> <b>must</b> be valid and unchanged for the life of
				this and all derived <code>ReferenceString</code>s.
*/
ReferencedString::ReferencedString(const std::string &str, size_t offset, size_t subSize)
	:_buffer(_substringAddress(str, offset, subSize)),
		_size(_substringSize(str, offset, subSize)) {trace_scope
}
/**
	@param other	The other string to get a substring of.
	@param offset	The offset in <code>other</code>. Defaults to the beginning.
	@param subSize	The number of bytes to references. Defaults to npos, or the rest of the string.
*/
inline ReferencedString::ReferencedString(const ReferencedString &other, size_t offset, size_t subSize)
	:_buffer(_substringAddress(other, offset, subSize)),
		_size(_substringSize(other, offset, subSize)) {trace_scope
}
/** Does nothing. */
inline ReferencedString::~ReferencedString() {trace_scope}
// @}
/** @name operators
	Helper functions to make it easier to use in C++.
*/ //@{
/**
	@param other	The string to copy.
	@return			Reference to <code>this</code>.
*/
inline ReferencedString &ReferencedString::operator=(const ReferencedString &other) {trace_scope
	_buffer= other._buffer;
	_size= other._size;
	return *this;
}
/** See valid().
	@return	<code>true</code> if this string is not empty.
*/
inline ReferencedString::operator bool() const {trace_scope
	return valid();
}
/**
	@param index	The index into the string.
	@return			A reference to the character at the given index, or a references to a null
						character if index is out of bounds.
*/
inline const char &ReferencedString::operator[](size_t index) const {trace_scope
	static const char	kNull= '\0';

	if( trace_bool(index >= _size) || trace_bool(NULL == _buffer) || trace_bool(_size <= 0) ) {trace_scope
		return kNull;
	}
	return getRaw(index);
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if the strings are identical.
*/
inline bool ReferencedString::operator==(const ReferencedString &str) const {trace_scope
	if(_size != str._size) {trace_scope
		return false;
	}
	if(_buffer == str._buffer) {trace_scope
		return true;
	}
	return compare(str) == 0;
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if the strings are not identical.
*/
inline bool ReferencedString::operator!=(const ReferencedString &str) const {trace_scope
	if(_size != str._size) {trace_scope
		return true;
	}
	if(_buffer == str._buffer) {trace_scope
		return false;
	}
	return compare(str) != 0;
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is less.
*/
inline bool ReferencedString::operator<(const ReferencedString &str) const {trace_scope
	return compare(str) < 0;
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is greater.
*/
inline bool ReferencedString::operator>(const ReferencedString &str) const {trace_scope
	return compare(str) > 0;
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is less or identical.
*/
inline bool ReferencedString::operator<=(const ReferencedString &str) const {trace_scope
	return compare(str) <= 0;
}
/** See compare(const ReferencedString&).
	@param str	The string to compare.
	@return		<code>true</code> if this string is greater or identical.
*/
inline bool ReferencedString::operator>=(const ReferencedString &str) const {trace_scope
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
inline const char &ReferencedString::getRaw(size_t index) const {trace_scope
	return reinterpret_cast<const char*>(_buffer)[index];
}
/** Copies the bytes into a std::string.
	@return	the std::string with the byte contents of <code>this</code>.
*/
inline std::string ReferencedString::string() const {trace_scope
	return std::string(&(*this)[0], _size);
}
/** Gets the raw pointer to the beginning of <code>this</code>.
	@return	The address of the buffer, or NULL if the string is empty.
*/
inline const char * const ReferencedString::data() const {trace_scope
	if(_size > 0) {trace_scope
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
inline ReferencedString &ReferencedString::assign(const ReferencedString &str, size_t offset, size_t subSize) {trace_scope
	if( trace_bool(offset < str._size) && trace_bool(trace_bool(npos == subSize) || trace_bool(subSize + offset > str._size)) ) {trace_scope
		subSize= str._size - offset;
	}
	if( trace_bool(offset >= str._size) || trace_bool(subSize <= 0) ) {trace_scope
		_buffer= NULL;
		_size= 0;
	} else {trace_scope
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
inline ReferencedString ReferencedString::substring(size_t offset, size_t subSize) const {trace_scope
	if( trace_bool(offset < _size) && trace_bool(trace_bool(npos == subSize) || trace_bool(offset + subSize > _size)) ) {trace_scope
		subSize= _size - offset;
	}
	if( trace_bool(NULL == _buffer) || trace_bool(0 == _size)
		|| trace_bool(offset >= _size)
		|| trace_bool(subSize <= 0) ) {trace_scope
		return ReferencedString();
	}
	return ReferencedString(&reinterpret_cast<const char*>(_buffer)[offset], subSize);
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
inline bool ReferencedString::sameAddress(const ReferencedString &str) const {trace_scope
	return _buffer == str._buffer;
}
/** See size(). For coding ease, depending on what language/paradigm you come from.
	@return	The number of bytes in <code>this</code>.
*/
inline size_t ReferencedString::size() const {trace_scope
	return _size;
}
/** See length(). For coding ease, depending on what language/paradigm you come from.
	@return	The number of bytes in <code>this</code>.
*/
inline size_t ReferencedString::length() const {trace_scope
	return _size;
}
/**
	@return	<code>true</code> if there is a buffer and we are referencing one or more bytes of it.
*/
inline bool ReferencedString::valid() const {trace_scope
	return trace_bool(NULL != _buffer) && trace_bool(_size > 0);
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
inline int ReferencedString::compare(const ReferencedString &str) const {trace_scope
	const unsigned char	*s1= reinterpret_cast<const unsigned char*>(_buffer);
	const unsigned char	*s2= reinterpret_cast<const unsigned char*>(str._buffer);

	for(size_t index= 0; trace_bool(index < _size) && trace_bool(index < str._size); ++index) {trace_scope
		if(s1[index] < s2[index]) {trace_scope
			return -1;
		} else if(s1[index] > s2[index]) {trace_scope
			return 1;
		}
	}
	if(_size < str._size) {trace_scope
		return -1;
	} else if(_size > str._size) {trace_scope
		return 1;
	}
	return 0;
}
/** If everything is removed, this string becomes invalid. See valid(). See operator bool().
	<code>this</code> is modified in place.
	@param count	The number of bytes to remove from the beginning of the string.
	@return			Reference to <code>this</code>.
*/
inline ReferencedString &ReferencedString::trimFromStart(size_t count) {trace_scope
	if(count >= _size) {trace_scope
		_buffer= NULL;
		_size= 0;
	} else {trace_scope
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
inline ReferencedString &ReferencedString::trimFromEnd(size_t count) {trace_scope
	if(count >= _size) {trace_scope
		_buffer= NULL;
		_size= 0;
	} else {trace_scope
		_size-= count;
	}
	return *this;
}
/** Short circuits the search if <code>this</code> is not valid or the offset is past the end.
	@param character	The byte to search for.
	@param offset		The position to start looking for <code>character</code>. Default is index 0.
*/
inline size_t ReferencedString::find(char character, size_t offset) const {trace_scope
	if( trace_bool(NULL == _buffer) || trace_bool(0 == _size) || trace_bool(offset >= _size) ) {trace_scope
		return npos;
	}
	return _find(character, offset);
}
/** Short circuits the search if <code>this</code> or <code>str</code> is not valid,
		or <code>str</code> could not fit in the space left after offset.
	@param str			The substring to search for.
	@param offset		The position to start looking for <code>str</code>. Default is index 0.
*/
inline size_t ReferencedString::find(const ReferencedString &str, size_t offset) const {trace_scope
	if( trace_bool(NULL == _buffer) || trace_bool(0 == _size)
		|| trace_bool(NULL == str._buffer) || trace_bool(0 == str._size)
		|| trace_bool(offset + str._size >= _size) ) {trace_scope
		return npos;
	}
	while(npos != offset) {trace_scope
		offset= _find(str[0], offset);
		if(npos != offset) {trace_scope
			if(substring(offset, str._size) == str) {trace_scope
				return offset;
			}
			++offset;
		}
	}
	return npos;
}
/** Core character search. _buffer and _size must be valid before calling.
	@param character	The byte to search for.
	@param offset		The position to start looking for <code>character</code>. Default is index 0.
*/
inline size_t ReferencedString::_find(char character, size_t offset) const {trace_scope
	const unsigned char	unsignedCharacter= static_cast<unsigned char>(character);
	const char			*start= reinterpret_cast<const char*>(_buffer);
	const char			*found= reinterpret_cast<const char*>(memchr(&start[offset],
																		unsignedCharacter,
																		_size - offset
																	)
															);

	if(NULL == found) {trace_scope
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
const void *ReferencedString::_substringAddress(const String &str, size_t offset, size_t subSize) {trace_scope
	if( trace_bool(NULL == str.data()) || trace_bool(str.size() <= 0) || trace_bool(offset >= str.size()) ) {trace_scope
		return NULL;
	}
	if( trace_bool(offset < str.size()) && trace_bool(trace_bool(npos == subSize) || trace_bool(offset + subSize > str.size())) ) {trace_scope
		subSize= str.size() - offset;
	}
	if( trace_bool(subSize <= 0) || trace_bool(offset >= str.size()) ) {trace_scope
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
size_t ReferencedString::_substringSize(const String &str, size_t offset, size_t subSize) {trace_scope
	if( trace_bool(NULL == str.data()) || trace_bool(str.size() <= 0) || trace_bool(offset >= str.size()) ) {trace_scope
		return 0;
	}
	if( trace_bool(offset < str.size()) && trace_bool(trace_bool(npos == subSize) || trace_bool(offset + subSize > str.size())) ) {trace_scope
		subSize= str.size() - offset;
	}
	if( trace_bool(subSize <= 0) || trace_bool(offset >= str.size()) ) {trace_scope
		return 0;
	}
	return subSize;
}

#endif // __ReferencedString_h__
