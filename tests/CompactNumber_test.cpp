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
	const uint64_t	testNumbers[]= {
		0, 1, 127, 128, 255, 256, 8191, 8192, 16383, 16384, 32767, 32768,
		1048575, 1048576, 2097151, 2097152, 4194303, 4194304, 8388607, 8388608,
		16777215, 16777216, 33554431, 33554432, 67108863, 67108864, 134217727, 134217728,
		268435455, 268435456, 536870911, 536870912, 1073741823, 173741824,
		2147483647, 2147483648, 4294967295, 4294967296
	};
	std::string	buffer(135, '\0');
	char		*pointer= const_cast<char*>(buffer.data());
	const char	*end= &pointer[buffer.size()];
	char				s1[100], s2[100];

	for(size_t i= 0; i < sizeof(testNumbers)/sizeof(testNumbers[0]); ++i) {
		if(!compactNumber::write(testNumbers[i], reinterpret_cast<void**>(&pointer), end)) {
			printf("-- WARNING: We have exceeded the buffer max\n");
		}
	}
	end= pointer;
	printf("%lu bytes compressed to %ld bytes\n", sizeof(testNumbers), pointer - const_cast<char*>(buffer.data()));
	pointer= const_cast<char*>(buffer.data());
	for(size_t i= 0; i < sizeof(testNumbers)/sizeof(testNumbers[0]); ++i) {
		uint64_t	value= compactNumber::read<uint64_t>(reinterpret_cast<const void**>(const_cast<const char**>(&pointer)), end);

		if(value != testNumbers[i]) {
			printf("FAILED: iteration %lu: value=%llu (0x%16llx,%s) testNumber=%llu (0x%16llx,%s)\n",
				i,
				value, value, bitPattern(value, s1),
				testNumbers[i], testNumbers[i], bitPattern(testNumbers[i], s2)
			);
		}
	}
	return 0;
}
