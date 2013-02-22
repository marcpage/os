#ifndef __BufferString_h__
#define __BufferString_h__

#include "Buffer.h"
#include <string>

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

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
	:_buffer(string) {trace_scope}
inline BufferString::~BufferString() {trace_scope}
/** @return <code>std::string.data()</code> */
inline void *BufferString::start() {trace_scope
	return reinterpret_cast<void*>(const_cast<char*>(_buffer.data()));
}
/** @return <code>std::string.size()</code> */
inline size_t BufferString::size() const {trace_scope
	return _buffer.size();
}

#endif // __BufferString_h__
