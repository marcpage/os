#ifndef __AsymmetricEncrypt_h__
#define __AsymmetricEncrypt_h__

/** @file AsymmetricEncrypt.h
	@todo document
	@todo genericize like Hash to move Mac specific code into platform isolcation
*/

#include "os/CryptoHelpers.h"
#include <openssl/pem.h>
#include <openssl/evp.h>

namespace crypto {

	class AsymmetricPublicKey {
		public:
			AsymmetricPublicKey();
			virtual ~AsymmetricPublicKey();
			virtual std::string &serialize();
			virtual bool verify(const std::string &text, const std::string &signature);
	};

	class AsymmetricPrivateKey {
		public:
			AsymmetricPrivateKey();
			virtual ~AsymmetricPrivateKey();
			virtual std::string &serialize();
			virtual std::string sign(const std::string &text);
	};

	template<typename T>
	class AutoClean {
		public:
			AutoClean(T *p=nullptr):data(p) {}
			~AutoClean() {if (nullptr != data) {dispose(); data = nullptr;}}
			operator const T*() const {return data;}
			operator T*() const {return data;}
			AutoClean<T> &operator=(const AutoClean<T> &other) {data = other.data; return *this;}
			void dispose() {free(data);}
			T *data;
	};

	template<>
	inline void AutoClean<RSA>::dispose() {RSA_free(data);}
	template<>
	inline void AutoClean<BIGNUM>::dispose() {BN_free(data);}
	template<>
	inline void AutoClean<BIO>::dispose() {BIO_free_all(data);}
	template<>
	inline void AutoClean<EVP_MD_CTX>::dispose() {EVP_MD_CTX_destroy(data);}
	template<>
	inline void AutoClean<EVP_PKEY>::dispose() {EVP_PKEY_free(data);}

	class OpenSSLRSA {
		public:
			typedef const EVP_MD*(*MessageDigestType)(void);
			OpenSSLRSA(const int keySize, const unsigned long publicExponent = RSA_F4) // publicExponent = RSA_3
					:_key(__crypto_OSSLHandle(RSA_new())), _publicExponent(__crypto_OSSLHandle(BN_new())) {
				__crypto_OSSLHandle(BN_set_word(_publicExponent, publicExponent));
				__crypto_OSSLHandle(RSA_generate_key_ex(_key, keySize, _publicExponent, nullptr));
			}
			typedef RSA*(*KeyReader)(BIO*,RSA**,pem_password_cb*,void*); // PEM_read_bio_RSAPublicKey or PEM_read_bio_RSAPrivateKey
			OpenSSLRSA(const std::string &serialized, KeyReader keyReader):_key(nullptr) {
				AutoClean<BIO> memory(__crypto_OSSLHandle(BIO_new_mem_buf(serialized.data(), serialized.size())));

				_key = __crypto_OSSLHandle(keyReader(memory, nullptr, nullptr, nullptr)); // exception thrown PEM routines:get_name:no start line
			}
			std::string &serializePrivate(std::string &buffer) {
				serializeKey(buffer, _writePrivate);
				return buffer;
			}
			std::string &serializePublic(std::string &buffer) {
				serializeKey(buffer, _writePublic); // crash EXC_BAD_ACCESS (code=1, address=0x80280ec0)
				return buffer;
			}
			std::string &sign(const std::string &text, std::string &signature, MessageDigestType messageDigestType) {  // EVP_sha256
				AutoClean<EVP_MD_CTX> signer(__crypto_OSSLHandle(EVP_MD_CTX_create()));
				AutoClean<EVP_PKEY> key(__crypto_OSSLHandle(EVP_PKEY_new()));
				size_t signatureSize = 0;

				__crypto_OSSLHandle(EVP_PKEY_assign_RSA(key, _key));
				__crypto_OSSLHandle(EVP_DigestSignInit(signer, nullptr, messageDigestType(), nullptr, key));
				__crypto_OSSLHandle(EVP_DigestSignUpdate(signer, text.data(), text.size()));
				__crypto_OSSLHandle(EVP_DigestSignFinal(signer, nullptr, &signatureSize)); // crash EXC_BAD_ACCESS (code=1, address=0x8)
				signature.assign(signatureSize, '\0');
				__crypto_OSSLHandle(EVP_DigestSignFinal(signer, reinterpret_cast<unsigned char*>(const_cast<char*>(signature.data())), &signatureSize));
				return signature; // binary signature
			}
			bool verify(const std::string &text, const std::string &signature, MessageDigestType messageDigestType) {  // EVP_sha256
				EVP_MD_CTX* verifier = (__crypto_OSSLHandle(EVP_MD_CTX_create())); // TODO: LEAKED
				AutoClean<EVP_PKEY> key(__crypto_OSSLHandle(EVP_PKEY_new()));
				int status = -1;

				__crypto_OSSLHandle(EVP_PKEY_assign_RSA(key, _key));
				__crypto_OSSLHandle(EVP_DigestVerifyInit(verifier, nullptr, messageDigestType(), nullptr, key));
				__crypto_OSSLHandle(EVP_DigestVerifyUpdate(verifier, text.data(), text.size()));
				status = EVP_DigestVerifyFinal(verifier, reinterpret_cast<const unsigned char *>(signature.data()), signature.size());
				if (status == 1) {
					return true;
				}
				if (status != 0) {
					handleOpenSSLResult(0, "EVP_DigestVerifyFinal", __FILE__, __LINE__);
				}
				return false;
			}
			~OpenSSLRSA() {
			}
		private:
			RSA* _key;
			AutoClean<BIGNUM> _publicExponent;
			typedef int(*KeyTypeSerializer)(BIO *b, RSA *r);
			static int _writePrivate(BIO *b, RSA *r) {
				return PEM_write_bio_RSAPrivateKey(b, r, nullptr, nullptr, 0, nullptr, nullptr);
			}
			static int _writePublic(BIO *b, RSA *r) {
				return PEM_write_bio_RSAPublicKey(b, r); // crash EXC_BAD_ACCESS (code=1, address=0x80280ec0)
			}
			void serializeKey(std::string &buffer, KeyTypeSerializer keytypeSerializer) {
				AutoClean<BIO> memory(__crypto_OSSLHandle(BIO_new(BIO_s_mem())));
				__crypto_OSSLHandle(keytypeSerializer(memory, _key)); // crash EXC_BAD_ACCESS (code=1, address=0x80280ec0)

				const int dataSize = BIO_pending(memory);

				buffer.assign(dataSize, '\0');
				__crypto_OSSLHandle(BIO_read(memory, const_cast<char*>(buffer.data()), dataSize));
			}
	};

}

#endif // __AsymmetricEncrypt_h__
