#include "os/SymetricEncrypt.h"
#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 500;
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

			fprintf(stderr, "AES256 size test\n");
			while(source.length() < 128) {
				fprintf(stderr, "test length=%lu\n", source.length());
				dotest(source == crypto::AES256(key).decrypt(crypto::AES256(key).encrypt(source)));
				fprintf(stderr, "%s\n", crypto::AES256(key).decrypt(crypto::AES256(key).encrypt(source)).c_str());
				source += "test";
			}
			fprintf(stderr, "AES256_EBC AlignmentError size test\n");
			try {
				crypto::AES256_EBC(key).decrypt(crypto::AES256_EBC(key).encrypt(source));
				fprintf(stderr, "FAILED: Exception should have happened\n");
			} catch(const crypto::AlignmentError &) {
			} catch(const std::exception &exception) {
				fprintf(stderr, "FAILED: Exception: %s\n", exception.what());
			}
			fprintf(stderr, "iv test\n");
			try {
				crypto::AES256(key).decryptWithIV(crypto::AES256(key).encryptWithIV(source, " "), " ");
				fprintf(stderr, "FAILED: Exception should have happened\n");
			} catch(const crypto::IVWrongSizeError &) {
			} catch(const std::exception &exception) {
				fprintf(stderr, "FAILED: Exception: %s\n", exception.what());
			}
			fprintf(stderr, "apple test\n");
		#if __APPLE_CC__ || __APPLE__
			try {
				encrypted.assign(2 * source.length() + crypto::CommonCrypto_AES256_CBC_Padded_Cryptor::BlockSize * 2, '\0');
				size_t encrypted_size = crypto::CommonCrypto_AES256_CBC_Padded_Cryptor::encrypt(key.data(), source.data(), source.length(), const_cast<char*>(encrypted.data()), encrypted.length(), NULL);
				encrypted.erase(encrypted_size);
				decrypted.assign(2048, '\0');
				crypto::CommonCrypto_AES256_CBC_Padded_Cryptor::decrypt(key.data(), encrypted.c_str(), encrypted.size(), const_cast<char*>(decrypted.data()), 5, NULL);
				fprintf(stderr, "FAILED: Exception should have happened\n");
			} catch(const crypto::BufferTooSmallError &) {
			} catch(const std::exception &exception) {
				fprintf(stderr, "FAILED: Exception: %s\n", exception.what());
			}
		#endif
			fprintf(stderr, "ParamError test\n");
			try {
				crypto::AES256(std::string("hello"));
				fprintf(stderr, "FAILED: bad key passed\n");
			} catch(const crypto::KeySizeError &) {
			} catch(const std::exception &exception) {
				fprintf(stderr, "FAILED: bad key Exception: %s\n", exception.what());
			}
			fprintf(stderr, "No Padding test\n");
			source = "";
			for (int iteration = 0; iteration < 256; ++iteration) {
				try {
					dotest(source == crypto::AES256_CBC(key).decrypt(crypto::AES256_CBC(key).encrypt(source)));
					if ( source.size() % 16 != 0) {
						fprintf(stderr, "FAILED: Iteration %d Exception: %s\n", iteration, "no error");
					}
				} catch(const crypto::AlignmentError &) {
					if ( source.size() % 16 == 0) {
						fprintf(stderr, "FAILED: Iteration %d Exception: %s\n", iteration, "no error");
					}
				} catch(const std::exception &exception) {
					fprintf(stderr, "FAILED: AES256_CBC Iteration %d Exception: %s\n", iteration, exception.what());
				}
				source += "a";
			}
			fprintf(stderr, "Testing permutations\n");
			dotest("test" == crypto::AES256_CBC_Padded(key).decrypt(crypto::AES256_CBC_Padded(key).encrypt("test")));
			dotest("test" == crypto::AES256_EBC_Padded(key).decrypt(crypto::AES256_EBC_Padded(key).encrypt("test")));
			dotest("1234567890123456" == crypto::AES256_CBC_Padded(key).decrypt(crypto::AES256_CBC_Padded(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_CBC(key).decrypt(crypto::AES256_CBC(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_EBC_Padded(key).decrypt(crypto::AES256_EBC_Padded(key).encrypt("1234567890123456")));
			dotest("1234567890123456" == crypto::AES256_EBC(key).decrypt(crypto::AES256_EBC(key).encrypt("1234567890123456")));
			fprintf(stderr, "Testing IV\n");
			iv = "1234567890123456";
			dotest(source == crypto::AES256(key).decryptWithIV(crypto::AES256(key).encryptWithIV(source, iv), iv));
		} catch(const std::exception &exception) {
			fprintf(stderr, "FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
