#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

// g++ -o /tmp/test tests/Hash_test.cpp -I. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 150000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		try {
			std::string	hash;
			hash::SpecificHash<hash::SHA256Hasher>	test("test", 4);

			dotest(std::string("sha256") == hash::SHA256Hasher().name());
			dotest(hash::SpecificHash<hash::SHA256Hasher>("test", 4) == hash::SpecificHash<hash::SHA256Hasher>("test", 4));
			dotest(hash::SpecificHash<hash::SHA256Hasher>("test", 4).hex(hash) == "f2ca1bb6c7e907d06dafe4687e579fce76b37e4e93b7605022da52e6ccc26fd2");
			fprintf(stderr, "FAIL %s\n", hash::SpecificHash<hash::SHA256Hasher>("test", 4).hex(hash).c_str());
			dotest(hash::SpecificHash<hash::SHA256Hasher>("test",4) == hash::SpecificHash<hash::SHA256Hasher>(hash.c_str()));
			dotest(hash::SpecificHash<hash::SHA256Hasher>(hash.c_str()).valid());
			dotest(hash::SpecificHash<hash::SHA256Hasher>("test", 4).valid());
			test= hash::SpecificHash<hash::SHA256Hasher>("hello", 5);
			dotest(hash::SpecificHash<hash::SHA256Hasher>("hello", 5) == test);
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
