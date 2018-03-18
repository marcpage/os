#include "os/SymetricEncrypt.h"
#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 1000;
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
			try {
				crypto::AES256_EBC(key).decrypt(crypto::AES256_EBC(key).encrypt(source));
				printf("FAILED: Exception should have happened\n");
			} catch(const crypto::AlignmentError &) {
			} catch(const std::exception &exception) {
				printf("FAILED: Exception: %s\n", exception.what());
			}
			try {
				crypto::AES256(key).decrypt(crypto::AES256(key).encrypt(source, " "), " ");
				printf("FAILED: Exception should have happened\n");
			} catch(const crypto::IVWrongSizeError &) {
			} catch(const std::exception &exception) {
				printf("FAILED: Exception: %s\n", exception.what());
			}
			try {
				encrypted = crypto::AES256(key).encrypt(source);
				decrypted.assign(2048, '\0');
				crypto::AES256(key).decrypt(encrypted.c_str(), encrypted.size(), const_cast<char*>(decrypted.data()), 5);
				printf("FAILED: Exception should have happened\n");
			} catch(const crypto::BufferTooSmallError &) {
			} catch(const std::exception &exception) {
				printf("FAILED: Exception: %s\n", exception.what());
			}
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
