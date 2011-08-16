#ifndef __EString_h__
#define __EString_h__

class EString {
	public:
		static const size_t	npos= static_cast<size_t>(-1);
		EString(const void *buffer= NULL, size_t bufferSize= 0)
			:_buffer(0 == bufferSize ? NULL : buffer), _size(NULL == buffer ? 0 : bufferSize) {
		}
		EString(const EString &other, size_t offset= 0, size_t subSize= npos)
			:_buffer(_substringAddress(other, offset, subSize)),
				_size(_substringSize(other, offset, subSize)) {
		}
		~EString() {}
		EString &operator=(const EString &other) {
			_buffer= other._buffer;
			_size= other.size();
			return *this;
		}
		operator bool() const {
			return valid();
		}
		const char &getRaw(size_t index) const {
			return reinterpret_cast<const char*>(_buffer)[index];
		}
		const char &operator[](size_t index) const {
			static const char	kNull= '\0';

			if( (index >= _size) || (NULL == _buffer) || (0 == _size) ) {
				return kNull;
			}
			return getRaw(index);
		}
		bool operator==(const EString &str) const {
			return compare(str) == 0;
		}
		bool operator!=(const EString &str) const {
			return compare(str) == 0;
		}
		bool operator<(const EString &str) const {
			return compare(str) < 0;
		}
		bool operator>(const EString &str) const {
			return compare(str) > 0;
		}
		bool operator<=(const EString &str) const {
			return compare(str) <= 0;
		}
		bool operator>=(const EString &str) const {
			return compare(str) >= 0;
		}
		std::string string() const {
			return std::string(&(*this)[0], size());
		}
		const char * const data() const {
			return reinterpret_cast<const char * const>(_buffer);
		}
		bool sameAddress(const EString &str) const {
			return _buffer == str._buffer;
		}
		size_t size() const {
			return _size;
		}
		size_t length() const {
			return _size;
		}
		bool valid() const {
			return (NULL != _buffer) && (0 != _size);
		}
		int compare(const EString &str) const {
			const unsigned char	*s1= reinterpret_cast<const unsigned char*>(_buffer);
			const unsigned char	*s2= reinterpret_cast<const unsigned char*>(str._buffer);

			for(size_t index= 0; (index < _size) && (index < str.size()); ++index) {
				if(s1[index] < s2[index]) {
					return -1;
				} else if(s1[index] > s2[index]) {
					return 1;
				}
			}
			if(_size < str.size()) {
				return -1;
			} else if(_size > str.size()) {
				return 1;
			}
			return 0;
		}
		EString &trimFromStart(size_t count) {
			if(count > size()) {
				_buffer= NULL;
				_size= 0;
			} else {
				_buffer= &(*this)[count];
				_size-= count;
			}
			return *this;
		}
		EString &trimFromEnd(size_t count) {
			if(count > size()) {
				_buffer= NULL;
				_size= 0;
			} else {
				_size-= count;
			}
			return *this;
		}
		EString &assign(const EString &str, size_t offset, size_t subSize= npos) {
			if(npos == subSize) {
				subSize= str.size() - offset;
			}
			if( (offset >= str.size()) || (subSize <= 0) || (subSize + offset > str.size()) ) {
				_buffer= NULL;
				_size= 0;
			} else {
				_buffer= &str[offset];
				_size= subSize;
			}
			return *this;
		}
		EString substring(size_t offset, size_t subSize= npos) const {
			if(npos == subSize) {
				subSize= _size - offset;
			}
			if( (NULL == _buffer) || (0 == _size)
				|| (offset > _size)
				|| (subSize <= 0) || (offset + subSize > _size) ) {
				return EString();
			}
			return EString(&reinterpret_cast<const char*>(_buffer)[offset], subSize);
		}
		size_t find(char character, size_t offset= 0) const {
			if( (NULL == _buffer) || (0 == _size) ) {
				return npos;
			}

			return _find(character, offset);
		}
		size_t find(const EString &str, size_t offset= 0) const {
			const size_t	kNpos= npos;

			if( (NULL == _buffer) || (0 == _size)
				|| (NULL == str._buffer) || (0 == str.size())
				|| (offset >= _size) ) {
				return kNpos;
			}
			while(kNpos != offset) {
				offset= _find(str[0], offset);
				if(kNpos != offset) {
					if(substring(offset, str.size()) == str) {
						return offset;
					}
					++offset;
				}
			}
			return kNpos;
		}
	private:
		const void *_buffer;
		size_t		_size;

		size_t _find(char character, size_t offset) const {
			const unsigned char	unsignedCharacter= static_cast<unsigned char>(character);
			const char			*start= reinterpret_cast<const char*>(_buffer);
			const char			*found= reinterpret_cast<const char*>(memchr(&start[offset],
																				unsignedCharacter,
																				size() - offset
																			)
																	);

			if(NULL == found) {
				return npos;
			}
			return found - start;
		}
		static const void *_substringAddress(const EString &str, size_t offset, size_t subSize) {
			if(npos == subSize) {
				subSize= str.size() - offset;
			}
			if( (NULL == str._buffer) || (0 == str.size())
				|| (offset >= str.size())
				|| (subSize <= 0) || (offset + subSize > str.size()) ) {
				return NULL;
			}
			return &reinterpret_cast<const char*>(str._buffer)[offset];
		}
		static size_t _substringSize(const EString &str, size_t offset, size_t subSize) {
			if(npos == subSize) {
				subSize= str.size() - offset;
			}
			if( (NULL == str._buffer) || (0 == str.size())
				|| (offset >= str.size())
				|| (subSize <= 0) || (offset + subSize > str.size()) ) {
				return 0;
			}
			return subSize;
		}
};

#endif // __EString_h__
