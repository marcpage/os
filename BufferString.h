#ifndef __BufferString_h__
#define __BufferString_h__

#include "Buffer.h"
#include <string>

/** Adapts a std::string to the Buffer interface.
	@note If the referenced string changes, <code>start()</code> and <code>size()</code>
		may no longer be valid.
	@todo Test!
*/
class BufferString : public Buffer {
	public:
		/// @brief Holds the references to the string.
		BufferString(std::string &string);
		/// @brief noop.
		virtual ~BufferString();
		/// @brief <code>data()</code>
		virtual void *start();
		/// @brief <code>size()</code>
		virtual size_t size() const;
	private:
		/// @brief Reference to the string
		std::string	&_buffer;
};

/** @param string	The string to adapt to Buffer interface. */
inline BufferString::BufferString(std::string &string)
	:_buffer(string) {}
inline BufferString::~BufferString() {}
/** @return <code>std::string.data()</code> */
inline void *BufferString::start() {
	return reinterpret_cast<void*>(const_cast<char*>(_buffer.data()));
}
/** @return <code>std::string.size()</code> */
inline size_t BufferString::size() const {
	return _buffer.size();
}

#endif // __BufferString_h__
