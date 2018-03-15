#include <stdio.h>
#include <string>
#include "os/CompactNumber.h"

static char *bitPattern(uint64_t value, char *buffer) {
	for(size_t bit= 0; bit < 64; ++bit) {
		const bool	bitValue= (value & (1 << (63 - bit))) != 0;

		buffer[bit]= bitValue ? '1' : '0';
	}
	buffer[64]= '\0';
	return buffer;
}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	int	iterations= 250000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int iteration= 0; iteration < iterations; ++iteration) {
		const uint64_t	testNumbers[]= {
			0, 1, 127, 128, 255, 256, 8191, 8192, 16383, 16384, 32767, 32768, // 0 - 11
			1048575, 1048576, 2097151, 2097152, 4194303, 4194304, 8388607, 8388608, // 11 - 19
			16777215, 16777216, 33554431, 33554432, 67108863, 67108864, 134217727, 134217728, // 20 - 27
			268435455, 268435456, 536870911, 536870912, 1073741823, 173741824, // 28 - 33
			2147483647, 2147483648UL, 4294967295UL, 4294967296ULL, // 29 - 32
			16511, 16512, 2113663, 2113664, 270549119, 270549120 // 33 - 38
		};
		uint32_t	values[]= {
						0, 127, 128,
						16511, 16512,
						2113663, 2113664,
						270549119, 270549120
		};
		uint8_t		compacts[]= {
						0x00, 0x7F, 0x80,0x00, // 0 - 3
						0xFF,0x7F, 0x80,0x80,0x00, // 4 - 8
						0xFF,0xFF,0x7F, 0x80,0x80,0x80,0x00, // 9 - 15
						0xFF,0xFF,0xFF,0x7F, 0x80,0x80,0x80,0x80,0x00, // 10 - 18
		};
		std::string	buffer(151, '\0');
		char		*pointer= const_cast<char*>(buffer.data());
		const char	*end= &pointer[buffer.size()];
		char		s1[100], s2[100];

		for(size_t i= 0; i < sizeof(testNumbers)/sizeof(testNumbers[0]); ++i) {
			if(!compactNumber::write(testNumbers[i], reinterpret_cast<void**>(&pointer), end)) {
				printf("-- WARNING: We have exceeded the buffer max\n");
			}
		}
		end= pointer;
		printf("%lu bytes compressed to %ld bytes\n", sizeof(testNumbers), static_cast<long>(pointer - const_cast<char*>(buffer.data())));
		pointer= const_cast<char*>(buffer.data());
		for(size_t i= 0; i < sizeof(testNumbers)/sizeof(testNumbers[0]); ++i) {
			uint64_t	value= compactNumber::read<uint64_t>(reinterpret_cast<const void**>(const_cast<const char**>(&pointer)), end);

			if(value != testNumbers[i]) {
				printf("FAILED: iteration %lu: value=%llu (0x%016llx,%s) testNumber=%llu (0x%016llx,%s)\n",
					i,
					value, value, bitPattern(value, s1),
					testNumbers[i], testNumbers[i], bitPattern(testNumbers[i], s2)
				);
			}
		}
		pointer= const_cast<char*>(buffer.data());
		for(size_t i= 0; i < sizeof(values)/sizeof(values[0]); ++i) {
			if(!compactNumber::write(values[i], reinterpret_cast<void**>(&pointer), end)) {
				printf("-- WARNING: We have exceeded the buffer max for values[%d]\n", static_cast<int>(i));
			}
		}
		if(pointer - const_cast<char*>(buffer.data()) != sizeof(compacts)) {
			printf("FAILED: we didn't write the number of bytes we thought we should: wrote=%d expected=%d\n",
				static_cast<int>(pointer - const_cast<char*>(buffer.data())),
				static_cast<int>(sizeof(compacts))
			);
		}
		for(size_t i= 0; i < sizeof(compacts)/sizeof(compacts[0]); ++i) {
			if(buffer.data()[i] != static_cast<char>(compacts[i])) {
				printf("FAILED: byte[%d] should be %02x but it is %02x\n",
					static_cast<int>(i), static_cast<int>(compacts[i]), static_cast<uint8_t>(buffer.data()[i])
				);
			}
		}
		pointer= const_cast<char*>(buffer.data());
		for(size_t i= 0; i < sizeof(values)/sizeof(values[0]); ++i) {
			if(values[i] != compactNumber::read<uint32_t>(reinterpret_cast<const void**>(const_cast<const char**>(&pointer)), end)) {
				printf("FAILED: value[%ld] didn't match\n", i);
			}
		}
	}
	return 0;
}
