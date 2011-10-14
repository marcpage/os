#ifndef __BufferAddress_h__
#define __BufferAddress_h__

#include "Buffer.h"
#include <string>

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

/** Adapt an arbitrary buffer to the Buffer protocol. */
class BufferAddress : public Buffer {
	public:
		/// @brief Adapt an Arbitrary Buffer
		BufferAddress(void *buffer, size_t bufferSize);
		/// @brief Adapt a null-character-terminated C-String
		BufferAddress(const char *stringBuffer);
		/// @brief noop
		virtual ~BufferAddress();
		/// @brief The address of the buffer
		virtual void *start();
		/// @brief The size of the buffer
		virtual size_t size() const;
	private:
		/// @brief The address of the buffer
		void	*_buffer;
		/// @brief The size of the buffer
		size_t	_size;
		BufferAddress(const BufferAddress&); ///< @brief Prevent Usage
		BufferAddress &operator=(const BufferAddress&); ///< @brief Prevent Usage
};

/**
	@param buffer		The buffer address to reference
	@param bufferSize	The bytes available in the buffer
*/
inline BufferAddress::BufferAddress(void *buffer, size_t bufferSize)
	:_buffer(buffer), _size(bufferSize) {trace_scope}
/** Uses strlen to get the buffer size, which does <b>not</b> include the null termination character.
	<b>WARNING</b> Make sure you do not attempt to write into this buffer if you pass in a string literal constant.
	@param stringBuffer		The null-character-terminated C-String address to reference
*/
inline BufferAddress::BufferAddress(const char *stringBuffer)
	:_buffer(const_cast<char*>(stringBuffer)), _size(::strlen(stringBuffer)) {trace_scope}
inline BufferAddress::~BufferAddress() {trace_scope}
/** @return the address of the buffer */
inline void *BufferAddress::start() {trace_scope
	return _buffer;
}
/** @return the number of bytes in the buffer. */
inline size_t BufferAddress::size() const {trace_scope
	return _size;
}

#endif // __BufferAddress_h__
