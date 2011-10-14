#ifndef __BufferManaged_h__
#define __BufferManaged_h__

#include "BufferAddress.h"
#include <string>

#ifndef trace_scope
	#define trace_scope ///< @brief in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< @brief in case Tracer.h is not included
#endif

/** Allocates (and deallocates) a buffer. */
class BufferManaged : public BufferAddress {
	public:
		/// @brief Allocates a buffer of the given size.
		BufferManaged(size_t size);
		/// @brief Deallocates the buffer
		virtual ~BufferManaged();
};

/** @param allocateSize The number of bytes to allocate for the buffer. */
inline BufferManaged::BufferManaged(size_t allocateSize)
	:BufferAddress(malloc(allocateSize), allocateSize) {trace_scope}
inline BufferManaged::~BufferManaged() {trace_scope
	free(BufferAddress::start());
}

#endif // __BufferManaged_h__
