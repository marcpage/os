#include <stdio.h>
#include "os/ZCompression.h"

int main(const int , const char * const []) {
	int	iterations= 100;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	const char * testCases[] = {
		" ", ".", "~",
		"test", "testing",
		"t", "tt", "ttt", "tttt", "ttttt", "tttttt", "ttttttt", "tttttttt", "ttttttttt",
		"The quick brown fox jumped over the lazy dog. And then the dog jumped up and bit the fox.",
		""
	};
	std::string	source;
	std::string compressed;
	std::string uncompressed;

	for (int i = 0; i < iterations; ++i) {
		for (unsigned int index = 0; index < sizeof(testCases)/sizeof(testCases[0]); ++index) {
			source = testCases[index];
			z::compress(source, compressed);
			z::uncompress(compressed, uncompressed);
			if (uncompressed != source) {
				printf("FAILED: iteration %d[%d] source '%s' -> '%s'\n", index, i, source.c_str(), uncompressed.c_str());
			}
		}
		while (source.size() < 1024 * 1024) {
			source += "Nobody inspects the spammish repetition. ";
		}
		try {
			z::compress(source, compressed);
			z::uncompress(compressed, uncompressed);
			printf("FAILED: We should have thrown and exception for size being to small\n");
		} catch(const z::Exception &) {
		} catch(const std::exception &exception) {
			printf("FAILED: We threw an unexpected exception: %s\n", exception.what());
		}
		z::compress(source, compressed);
		z::uncompress(compressed, uncompressed, source.size());
			if (uncompressed != source) {
				printf("FAILED: \n---- source ----\n%s\n------ destination -----\n%s\n------------\n", source.c_str(), uncompressed.c_str());
			}
	}
	return 0;
}
