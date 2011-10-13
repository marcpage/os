#ifndef __BufferString_h__
#define __BufferString_h__

#include "Buffer.h"
#include <string>

class BufferString : public Buffer {
	public:
		BufferString(std::string &string);
		virtual ~BufferString();
		virtual void *start();
		virtual size_t size();
	private:
		std::string	&_buffer;
};

inline BufferString::BufferString(std::string &string)
	:_buffer(string) {}
inline BufferString::~BufferString() {}
inline void *BufferString::start() {
	return _buffer.data();
}
inline size_t BufferString::size() {
	return _buffer.size();
}

#endif // __BufferString_h__
