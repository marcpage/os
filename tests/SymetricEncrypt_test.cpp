#include "os/SymetricEncrypt.h"
#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 100;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		try {
			hash::sha256	keyData("key", 3);
			std::string		key(reinterpret_cast<const char*>(keyData.buffer()), keyData.size());
			crypto::AES256 	cryptor(key);
			std::string		source = "test ";
			std::string		encrypted;
			std::string		decrypted;

			while(source.length() < 1024) {
				dotest(source == crypto::AES256(key).decrypt(crypto::AES256(key).encrypt(source)));
				source += "test";
			}
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
