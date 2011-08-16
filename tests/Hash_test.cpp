#include "Hash.h"
#include <stdio.h>

// g++ -o /tmp/test posix/tests/Hash_test.cpp -Iposix -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

int main(int /*argc*/, char * /*argv*/[]) {
	try {
		std::string	hash;
		Hash<SHA256Hasher>	test("test", 4);
		
		if(Hash<SHA256Hasher>("test", 4) != Hash<SHA256Hasher>("test", 4)) {
			printf("Test of hash failed\n");
		}
		Hash<SHA256Hasher>("test", 4).hex(hash);
		if(Hash<SHA256Hasher>("test",4) != Hash<SHA256Hasher>(hash.c_str())) {
			printf("Hex and back failed: %s\n", hash.c_str());
		}
		if(!Hash<SHA256Hasher>(hash.c_str()).valid()) {
			printf("Hash of hex not valid\n");
		}
		if(!Hash<SHA256Hasher>("test", 4).valid()) {
			printf("Hash of data not valid\n");
		}
		test= Hash<SHA256Hasher>("hello", 5);
		if(Hash<SHA256Hasher>("hello", 5) != test) {
			printf("failed hello assignment test\n");
		}
	} catch(const std::exception &exception) {
		printf("FAILED: Exception: %s\n", exception.what());
	}
	return 0;
}
