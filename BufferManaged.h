#ifndef __BufferManaged_h__
#define __BufferManaged_h__

#include "BufferAddress.h"
#include <string>

class BufferManaged : public BufferAddress {
	public:
		BufferString(size_t size);
		virtual ~BufferString();
};

inline BufferManaged::BufferManaged(size_t size)
	:BufferAddress(malloc(size), size) {}
inline BufferManaged::~BufferManaged() {
	free(BufferAddress::start());
}

#endif // __BufferManaged_h__
