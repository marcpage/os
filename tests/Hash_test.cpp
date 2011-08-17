#include "Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

// g++ -o /tmp/test tests/Hash_test.cpp -I. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

int main(int /*argc*/, char * /*argv*/[]) {
	try {
		std::string	hash;
		hash::SpecificHash<hash::SHA256Hasher>	test("test", 4);

		dotest(std::string("sha256") == hash::SHA256Hasher().name());
		dotest(hash::SpecificHash<hash::SHA256Hasher>("test", 4) == hash::SpecificHash<hash::SHA256Hasher>("test", 4));
		hash::SpecificHash<hash::SHA256Hasher>("test", 4).hex(hash);
		dotest(hash::SpecificHash<hash::SHA256Hasher>("test",4) == hash::SpecificHash<hash::SHA256Hasher>(hash.c_str()));
		dotest(hash::SpecificHash<hash::SHA256Hasher>(hash.c_str()).valid());
		dotest(hash::SpecificHash<hash::SHA256Hasher>("test", 4).valid());
		test= hash::SpecificHash<hash::SHA256Hasher>("hello", 5);
		dotest(hash::SpecificHash<hash::SHA256Hasher>("hello", 5) == test);
	} catch(const std::exception &exception) {
		printf("FAILED: Exception: %s\n", exception.what());
	}
	return 0;
}
