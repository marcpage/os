#ifndef __BufferManaged_h__
#define __BufferManaged_h__

#include "BufferAddress.h"
#include <string>

/** Allocates (and deallocates) a buffer.
	@todo Test!
*/
class BufferManaged : public BufferAddress {
	public:
		/// @brief Allocates a buffer of the given size.
		BufferManaged(size_t size);
		/// @brief Deallocates the buffer
		virtual ~BufferManaged();
};

/** @param allocateSize The number of bytes to allocate for the buffer. */
inline BufferManaged::BufferManaged(size_t allocateSize)
	:BufferAddress(malloc(allocateSize), allocateSize) {}
inline BufferManaged::~BufferManaged() {
	free(BufferAddress::start());
}

#endif // __BufferManaged_h__
