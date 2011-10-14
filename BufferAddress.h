#ifndef __BufferAddress_h__
#define __BufferAddress_h__

#include "Buffer.h"
#include <string>

class BufferAddress : public Buffer {
	public:
		BufferAddress(void *buffer, size_t size);
		virtual ~BufferAddress();
		virtual void *start();
		virtual size_t size() const;
	private:
		void	*_buffer;
		size_t	_size;
		BufferAddress(const BufferAddress&); ///< Prevent Usage
		BufferAddress &operator=(const BufferAddress&); ///< Prevent Usage
};

inline BufferAddress::BufferAddress(void *buffer, size_t bufferSize)
	:_buffer(buffer), _size(bufferSize) {}
inline BufferAddress::~BufferAddress() {}
inline void *BufferAddress::start() {
	return _buffer;
}
inline size_t BufferAddress::size() const {
	return _size;
}

#endif // __BufferAddress_h__
