#ifndef __BufferManaged_h__
#define __BufferManaged_h__

#include "BufferAddress.h"
#include <string>

class BufferManaged : public BufferAddress {
	public:
		BufferManaged(size_t size);
		virtual ~BufferManaged();
};

inline BufferManaged::BufferManaged(size_t allocateSize)
	:BufferAddress(malloc(allocateSize), allocateSize) {}
inline BufferManaged::~BufferManaged() {
	free(BufferAddress::start());
}

#endif // __BufferManaged_h__
