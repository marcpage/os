#include <stdio.h>
#include "../Library.h"

// g++ Library_call_test.cpp -o /tmp/test -framework Carbon -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -Wno-empty-body

typedef uintptr_t (*Call9)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

/*
ZEXTERN const char * ZEXPORT zlibVersion OF((void));
ZEXTERN uLong ZEXPORT zlibCompileFlags OF((void));
ZEXTERN int ZEXPORT compress OF((Bytef *dest, uLongf *destLen,
                                 const Bytef *source, uLong sourceLen));
ZEXTERN int ZEXPORT compress2 OF((Bytef *dest, uLongf *destLen,
                                  const Bytef *source, uLong sourceLen,
                                  int level));
ZEXTERN uLong ZEXPORT compressBound OF((uLong sourceLen));
ZEXTERN int ZEXPORT uncompress OF((Bytef *dest, uLongf *destLen,
                                   const Bytef *source, uLong sourceLen));
*/
int main(const int argc, const char * const argv[]) {
	sys::Library	z("z");
	const char * const	original= "Testing compression";
	unsigned char buffer[4096];
	char result[4096];

	uintptr_t	version= z.function<Call9>("zlibVersion")(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	uintptr_t	flags= z.function<Call9>("zlibCompileFlags")(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	uintptr_t	bounds= z.function<Call9>("compressBound")(strlen(original), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	uintptr_t	compressResults= z.function<Call9>("compress2")(reinterpret_cast<uintptr_t>(buffer),
	return 0;
}
