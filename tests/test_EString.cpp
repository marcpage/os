#include <stdio.h>
#include "../EString.h"

// g++ -o /tmp/test test_EString.cpp -I.. -include ../tracer.h -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -isysroot $SDK_PATH/MacOSX10.5.sdk
// /tmp/test | sort | grep EString.h | uniq | wc -l

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int argc, const char * const argv[]) { // coverage 98
	std::string	buffer("testing more stuff now test");
	EString		contents(buffer.data(), buffer.size());
	EString		word1, word2, word3, word4, word5;
	size_t		offset1= contents.find(' '), offset2;
	const std::string	testing("testing");
	const std::string	more("more");
	const std::string	stuff("stuff");
	const std::string	now("now");
	const std::string	test("test");

	dotest(contents.size() == contents.length());
	dotest(contents.size() == 27);
	dotest(!word1.valid()); dotest(!word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(offset1 == 7);
	word1= contents.substring(0, offset1);
	dotest(word1.valid()); dotest(!word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	word2= word1;
	dotest(word1.valid()); dotest(word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(word1 == word2);
	dotest(word1 == testing);
	dotest(word2 == testing);

	offset2= contents.find(' ', offset1 + 1);
	dotest(offset2 == 12);
	word2= EString(buffer, offset1 + 1, offset2 - offset1 - 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(word1 != word2);
	dotest(word2 == more);

	offset1= contents.find(' ', offset2 + 1);
	dotest(offset1 == 18);
	word3.assign(buffer, offset2 + 1, offset1 - offset2 - 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(word1 != word3);
	dotest(word2 != word3);
	dotest(word3 == stuff);

	offset2= contents.find(' ', offset1 + 1);
	dotest(offset2 == 22);
	word4= contents.substring(offset1 + 1, offset2 - offset1 - 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(word3.valid()); dotest(word4.valid()); dotest(!word5.valid());
	dotest(word1 != word4);
	dotest(word2 != word4);
	dotest(word3 != word4);
	dotest(word4 == now);

	word5= contents.substring(offset2 + 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(word3.valid()); dotest(word4.valid()); dotest(word5.valid());
	dotest(word1 != word5);
	dotest(word2 != word5);
	dotest(word3 != word5);
	dotest(word4 != word5);
	dotest(word5 == test);
	printf("word2='%s' word5='%s'\n",word2.string().c_str(),word5.string().c_str());

	dotest(word1 == word1);
	dotest(word1 > word2);
	dotest(word1 > word3);
	dotest(word1 > word4);
	dotest(word1 > word5);

	dotest(word2 < word1);
	dotest(word2 == word2);
	dotest(word2 < word3);
	dotest(word2 < word4);
	dotest(word2 < word5);

	dotest(word3 < word1);
	dotest(word3 > word2);
	dotest(word3 == word3);
	dotest(word3 > word4);
	dotest(word3 < word5);

	dotest(word4 < word1);
	dotest(word4 > word2);
	dotest(word4 < word3);
	dotest(word4 == word4);
	dotest(word4 < word5);

	dotest(word5 < word1);
	dotest(word5 > word2);
	dotest(word5 > word3);
	dotest(word5 > word4);
	dotest(word5 == word5);

	dotest(word1 >= word1);
	dotest(word1 <= word1);
	dotest(word1 >= word2);
	dotest(word1 >= word3);
	dotest(word1 >= word4);
	dotest(word1 >= word5);

	dotest(word2 <= word1);
	dotest(word2 >= word2);
	dotest(word2 <= word2);
	dotest(word2 <= word3);
	dotest(word2 <= word4);
	dotest(word2 <= word5);

	dotest(word3 <= word1);
	dotest(word3 >= word2);
	dotest(word3 >= word3);
	dotest(word3 <= word3);
	dotest(word3 >= word4);
	dotest(word3 <= word5);

	dotest(word4 <= word1);
	dotest(word4 >= word2);
	dotest(word4 <= word3);
	dotest(word4 >= word4);
	dotest(word4 <= word4);
	dotest(word4 <= word5);

	dotest(word5 <= word1);
	dotest(word5 >= word2);
	dotest(word5 >= word3);
	dotest(word5 >= word4);
	dotest(word5 >= word5);
	dotest(word5 <= word5);
	return 0;
}
