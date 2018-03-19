#include "os/SymetricEncrypt.h"
#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 300;
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
			std::string		iv;

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
			try {
				crypto::AES256(std::string("hello"));
				printf("FAILED: bad key passed\n");
			} catch(const crypto::ParamError &) {
			} catch(const std::exception &exception) {
				printf("FAILED: bad key Exception: %s\n", exception.what());
			}
			source = "";
			for (int iteration = 0; iteration < 256; ++iteration) {
				try {
					dotest(source == crypto::AES256_CBC(key).decrypt(crypto::AES256_CBC(key).encrypt(source)));
					if ( source.size() % 16 != 0) {
						printf("FAILED: Iteration %d Exception: %s\n", iteration, "no error");
					}
				} catch(const crypto::AlignmentError &) {
					if ( source.size() % 16 == 0) {
						printf("FAILED: Iteration %d Exception: %s\n", iteration, "no error");
					}
				} catch(const std::exception &exception) {
					printf("FAILED: AES256_CBC Iteration %d Exception: %s\n", iteration, exception.what());
				}
				source += "a";
			}
			dotest("test" == crypto::AES256_CBC_Padded(key).decrypt(crypto::AES256_CBC_Padded(key).encrypt("test")));
			dotest("test" == crypto::AES256_EBC_Padded(key).decrypt(crypto::AES256_EBC_Padded(key).encrypt("test")));
			dotest("1234567890123456" == crypto::AES256_CBC_Padded(key).decrypt(crypto::AES256_CBC_Padded(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_CBC(key).decrypt(crypto::AES256_CBC(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_EBC_Padded(key).decrypt(crypto::AES256_EBC_Padded(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_EBC(key).decrypt(crypto::AES256_EBC(key).encrypt("1234567890123456")));
			iv = "1234567890123456";
			dotest(source == crypto::AES256(key).decrypt(crypto::AES256(key).encrypt(source, iv), iv));
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
