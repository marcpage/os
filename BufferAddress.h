#ifndef __BufferAddress_h__
#define __BufferAddress_h__

#include "Buffer.h"
#include <string>

class BufferAddress : public Buffer {
	public:
		BufferString(void *buffer, size_t size);
		virtual ~BufferString();
		virtual void *start();
		virtual size_t size();
	private:
		void	*_buffer;
		size_t	_size;
};

inline BufferAddress::BufferAddress(void *buffer, size_t size)
	:_buffer(buffer), _size(size) {}
inline BufferAddress::~BufferAddress() {}
inline void *BufferAddress::start() {
	return _buffer;
}
inline size_t BufferAddress::size() {
	return _size;
}

#endif // __BufferAddress_h__
