#include <stdio.h>
#include "os/ReferencedString.h"

// $ g++ -o /tmp/test tests/ReferencedString_test.cpp -isysroot $SDK_PATH/MacOSX10.5.sdk -I. -include Tracer.h -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	std::string			buffer("testing more stuff now test");
	ReferencedString	contents(buffer.data(), buffer.size());
	ReferencedString	word1(NULL), word2(NULL, 0), word3, word4, word5;
	ReferencedString	other1, other2, other3, other4, other5, other6;
	ReferencedString	moreRef("more");
	size_t				offset1= contents.find(' '), offset2;
	const std::string	testing("testing");
	const std::string	more("more");
	const std::string	stuff("stuff");
	const std::string	now("now");
	const std::string	test("test");

	dotest(word1[0] == '\0');
	dotest(contents[26] == 't');
	dotest(contents[27] == '\0');
	dotest(word1.data() == NULL);
	dotest(contents);
	dotest(!word1);
	dotest(contents.data()[0] == 't');
	dotest(moreRef == more);
	dotest(contents.size() == contents.length());
	dotest(contents.size() == 27);
	dotest(!word1.valid()); dotest(!word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(offset1 == 7);
	word1= contents.substring(0, offset1);
	dotest(word1.valid()); dotest(!word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(word1.sameAddress(contents));
	dotest(contents.sameAddress(word1));
	word2= word1;
	dotest(word1.valid()); dotest(word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(word1.sameAddress(word2));
	dotest(word2.sameAddress(word1));
	dotest(word1 == word2);
	dotest(word1 == testing);
	dotest(word2 == testing);

	offset2= contents.find(' ', offset1 + 1);
	dotest(offset2 == 12);
	word2= ReferencedString(buffer, offset1 + 1, offset2 - offset1 - 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(!word3.valid()); dotest(!word4.valid()); dotest(!word5.valid());
	dotest(!word1.sameAddress(word2));
	dotest(!word1.sameAddress(word3));
	dotest(!word3.sameAddress(word1));
	dotest(word3.sameAddress(word4));
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

	word5= ReferencedString(contents, offset2 + 1);
	dotest(word1.valid()); dotest(word2.valid()); dotest(word3.valid()); dotest(word4.valid()); dotest(word5.valid());
	dotest(word1 != word5);
	dotest(word2 != word5);
	dotest(word3 != word5);
	dotest(word4 != word5);
	dotest(word5 == test);
	printf("word2='%s' word5='%s'\n",word2.string().c_str(),word5.string().c_str());

	dotest( ! (word1 == word5) );
	dotest(word1 == word1);
	dotest( ! (word1 != word1) );

	other1= word1;
	dotest(other1.trimFromEnd(3) == word5);
	other2= word5;
	dotest(other1 == other2);
	other2.trimFromStart(2);
	dotest(other1 != other2);
	other3= word3;
	other3.trimFromEnd(3);
	dotest(other3 == other2);
	dotest(contents.find(other3) == 2);
	dotest(other4.assign(contents, 23) == test);
	dotest(!other5.assign(contents, 27).valid());
	dotest(!other6.assign(word1).trimFromStart(7).valid());
	dotest(!other6.assign(word1).trimFromEnd(7).valid());
	dotest(other6.assign(word1).trimFromStart(6) == "g");
	dotest(other6.assign(word1).trimFromEnd(6) == "t");
	dotest(contents.substring(23) == test);
	dotest(!ReferencedString().substring(5).valid());
	dotest(!word1.substring(7).valid());
	dotest(word1.substring(6) == "g");
	dotest(ReferencedString::npos == ReferencedString().find('.'));
	dotest(ReferencedString::npos == word1.find('.', 7));
	dotest(6 == word1.find('g', 6));
	dotest(ReferencedString().assign(word1,4,3) == "ing");
	dotest(ReferencedString().assign(word1,4,4) == "ing");
	dotest(ReferencedString().assign(word1,6) == "g");
	dotest(!ReferencedString().assign(word1,7).valid());
	dotest(!ReferencedString().assign(word1,4,0).valid());
	dotest(word1.substring(4,3) == "ing");
	dotest(word1.substring(4,4) == "ing");
	dotest(!word1.substring(4,0).valid());
	dotest(ReferencedString::npos == ReferencedString().find(word1));
	dotest(ReferencedString::npos == word1.find(ReferencedString()));
	dotest(ReferencedString::npos == word1.find("ing", 5));
	dotest(ReferencedString::npos == word1.find("e", 2));
	dotest(3 == word1.find("ting"));

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
