#include "../EString.h"
#include <stdio.h>

// g++ -o /tmp/test test_EString.cpp -I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings

#define test(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int argc, const char * const argv[]) {
	std::string	buffer("testing more stuff now test");
	EString		contents(buffer.data(), buffer.size());
	EString		word1, word2, word3, word4, word5;
	size_t		offset= contents.find(' ');

	word1= contents.substring(0, offset);
	word2= word1;
	test(word1 == word2);
	return 0;
}
