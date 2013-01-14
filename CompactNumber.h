#ifndef __CompactNumber_h__
#define __CompactNumber_h__

#include <stdint.h>

namespace compactNumber {

	template<typename Integer>
	Integer read(const void **buffer, const void *bufferEnd) {
		const size_t	IntegerBits= sizeof(Integer) * 8;
		const size_t	streamBitsPerByte= 7;
		const uint8_t	topBitMask= 0x80;
		const uint8_t	maskOutTopBit= static_cast<uint8_t>(~topBitMask);
		const size_t	maxIntegerStreamBytes= 1 + (IntegerBits - 1) / streamBitsPerByte;
		Integer			result= 0;
		const uint8_t	*buf= reinterpret_cast<const uint8_t*>(*buffer);
		const uint8_t	*end= reinterpret_cast<const uint8_t*>(bufferEnd);
		size_t			iterations= 0;

		while( buf != end ) {
			const uint8_t	positionValue= *buf;
			const bool		last= (positionValue & topBitMask) == 0;
			const uint8_t	byteValue= positionValue & maskOutTopBit;

			result= (result << streamBitsPerByte) | byteValue;
			++buf;
			if( last ) {
				break;
			}
			++iterations;
			if(iterations > maxIntegerStreamBytes) {
				return 0; // exceeded maximum size of Integer
			}
		}
		*buffer= reinterpret_cast<const void *>(buf);
		return result;
	}
	template<typename Integer>
	bool write(Integer integer, void **buffer, const void *bufferEnd) {
		const size_t	IntegerBits= sizeof(Integer) * 8;
		const size_t	streamBitsPerByte= 7;
		const uint8_t	topBitMask= 0x80;
		const uint8_t	maskOutTopBit= static_cast<uint8_t>(~topBitMask);
		const size_t	maxIntegerStreamBytes= 1 + (IntegerBits - 1) / streamBitsPerByte;
		uint8_t			*buf= reinterpret_cast<uint8_t*>(*buffer);
		const uint8_t	*end= reinterpret_cast<const uint8_t*>(bufferEnd);
		bool			started= false;

		if(*buffer == bufferEnd) {
			return false;
		}
		if(integer == 0) {
			*buf= 0;
			++buf;
		} else for(size_t streamByte= 0; streamByte < maxIntegerStreamBytes; ++streamByte) {
			const size_t	shift= 7*(maxIntegerStreamBytes - streamByte - 1);
			uint8_t	streamByteValue= (integer >> shift) & maskOutTopBit;

			if(buf == end) {
				return false;
			}
			started= started || (streamByteValue != 0);
			if(started) {
				if(streamByte < maxIntegerStreamBytes - 1) {
					streamByteValue|= topBitMask;
				}
				*buf= streamByteValue;
				++buf;
			}
		}
		*buffer= reinterpret_cast<void *>(buf);
		return true;
	}

}

#endif // __CompactNumber_h__
