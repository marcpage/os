#ifndef __PackedNumbers_h__
#define __PackedNumbers_h__

namespace pn {

	/** Big Endian packed number read.
	*/
	template<typename Int>
	Int unpack(const void *buffer, int &offset) {
		const unsigned char * const	buf= reinterpret_cast<const unsigned char * const>(buffer);
		Int	base= 0;
		Int	bits= 0;

		do	{
		} while(buf[offset]

	}
}

/*
	base= 1 << (7*bytes)
	bytes	base		max
	1		          0	           127
	2		        128	        16,511
	3		     16,512	     2,113,663
	4		  2,113,664	   270,549,119
	5		270,549,120	34,630,287,487
*/

#endif // __PackedNumbers_h__
