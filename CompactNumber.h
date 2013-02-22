#ifndef __CompactNumber_h__
#define __CompactNumber_h__

#include <stdint.h>

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

/** Reads and writes variables sized integers from/to a buffer.
	Numbers are stored in the lower 7 bits of bytes.
	The number of bytes determines what offset to add to the lower-7-bits pattern.
	For instance, for 1 byte you add 0, for 2 bytes, you add 128, for 3 bytes you add 16,512, etc.
	The last byte of any number has the upper bit cleared, all others have the high bit set.
	Numbers are stored in Big Endian format.
	<ul>
		<li> Numbers 0 through 127 are stored in 1 byte. (00 - 7F)
		<li> Numbers 128 through 16,511 are stored in 2 bytes. (80 00 - FF 7F)
		<li> Numbers 16,512 through 2,113,663 are stored in 3 bytes. (80 00 00 - FF FF 7F)
		<li> Numbers 2,113,664 through 270,549,119 are stored in 3 bytes. (80 00 00 00 - FF FF FF 7F)
		<li> etc.
	</ul>
*/
namespace compactNumber {

	/** Reads a compact number from a buffer.
		If a compact number could not be read, there are two scenarios:
			The compact number is too big for sizeof(Integer)
			or
			The compact number extends past the end of bufferEnd.
		This allows you to load partial buffers from disk or network
			and then retry after loading more of the buffer.
		If Integer is a class, it must implement:
		<ul>
			<li> Integer(int)
			<li> Integer(uint8_t)
			<li> Integer operator<<(size_t)
			<li> Integer operator|(uint8_t)
			<li> Integer &operator=(int)
			<li> Integer &operator+=(const Integer&)
		</ul>
		@param buffer		A pointer to the buffer pointer.
								If we were successful in reading a compact number,
								the pointer will be advanced just past the number.
								If NULL or *buffer is NULL, behavior is undefined.
		@param bufferEnd	No data will be read from the buffer pointer beyond this point.
								If NULL, behavior is undefined.
		@return				Zero if a complete compact number could not be read
								or it doesn't fit in sizeof(Integer), or if successful
								the value of the compact number.
		@todo Test *buffer == bufferEnd
		@todo Test reading number bigger than Integer
	*/
	template<typename Integer>
	Integer read(const void **buffer, const void *bufferEnd) {trace_scope
		const size_t	IntegerBits= sizeof(Integer) * 8;
		const size_t	streamBitsPerByte= 7;
		const uint8_t	topBitMask= 0x80;
		const uint8_t	maskOutTopBit= static_cast<uint8_t>(~topBitMask);
		const size_t	maxIntegerStreamBytes= 1 + (IntegerBits - 1) / streamBitsPerByte;
		Integer			result= 0;
		Integer			base= 0;
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
			base+= (Integer(topBitMask) << (streamBitsPerByte*(buf - reinterpret_cast<const uint8_t*>(*buffer) - 1)));
			++iterations;
			if(iterations > maxIntegerStreamBytes) {
				return 0; // exceeded maximum size of Integer
			}
		}
		*buffer= reinterpret_cast<const void *>(buf);
		return base + result;
	}

	/** Puts a number into a buffer.
		If a compact number could not be written
			then the compact number extends past the end of bufferEnd.
		This allows you to grow the buffer as needed.
		If Integer is a class, sizeof(Integer) must result in number of data bytes
			and it must implement:
		<ul>
			<li> Integer(uint8_t)
			<li> Integer operator>>(size_t)
			<li> Integer operator&(uint8_t)
			<li> bool operator==(int)
			<li> operator uint8_t()
			<li> Integer &operator-=(const Integer&)
			<li> Integer &operator=(const Integer&)
			<li> Integer &operator+=(const Integer&)
			<li> bool operator>(const Integer&)
			<li> Integer operator<<(size_t)
		</ul>
		@param integer		The number to write to the buffer as a compact number.
		@param buffer		A pointer to the buffer pointer.
								If we were successful in writing a compact number,
								the pointer will be advanced just past the number.
								If NULL or *buffer is NULL, behavior is undefined.
		@param bufferEnd	No data will be written to the buffer pointer beyond this point.
								If NULL, behavior is undefined.
		@return				true if we were able to write the number to the buffer
							or false if there is not enough room in the buffer.
		@todo Test *buffer == bufferEnd
	*/
	template<typename Integer>
	bool write(Integer integer, void **buffer, const void *bufferEnd) {trace_scope
		const size_t	IntegerBits= sizeof(Integer) * 8;
		const size_t	streamBitsPerByte= 7;
		const uint8_t	topBitMask= 0x80;
		const uint8_t	maskOutTopBit= static_cast<uint8_t>(~topBitMask);
		const size_t	maxIntegerStreamBytes= 1 + (IntegerBits - 1) / streamBitsPerByte;
		uint8_t			*buf= reinterpret_cast<uint8_t*>(*buffer);
		const uint8_t	*end= reinterpret_cast<const uint8_t*>(bufferEnd);
		Integer			partialBase= 0x80;
		size_t			bytesToWrite= 1;

		if(*buffer == bufferEnd) {
			return false;
		}
		while( ( integer >= partialBase) && (bytesToWrite < maxIntegerStreamBytes) ) {
			integer-= partialBase;
			partialBase= (Integer(topBitMask) << (streamBitsPerByte * bytesToWrite));
			++bytesToWrite;
		}
		for(size_t streamByte= 0; streamByte < bytesToWrite; ++streamByte) {
			const size_t	shift= streamBitsPerByte*(bytesToWrite - streamByte - 1);
			const uint8_t	streamByteRawValue= (integer >> shift) & maskOutTopBit;
			const bool		lastByte= (streamByte == bytesToWrite - 1);
			const uint8_t	streamByteValue= streamByteRawValue | (trace_bool(!lastByte) ? topBitMask : 0);

			if(buf == end) {
				return trace_bool(false);
			}
			*buf= streamByteValue;
			++buf;
		}
		*buffer= reinterpret_cast<void *>(buf);
		return trace_bool(true);
	}

}

#endif // __CompactNumber_h__
