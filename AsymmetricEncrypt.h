#ifndef __AsymmetricEncrypt_h__
#define __AsymmetricEncrypt_h__

/** @file AsymmetricEncrypt.h
	@todo document
	@todo genericize like Hash to move Mac specific code into platform isolcation
*/

#include "os/CryptoHelpers.h"

#if OpenSSLAvailable
	#include <openssl/pem.h>
	#include <openssl/evp.h>
#endif

namespace crypto {

	class AsymmetricPublicKey {
		public:
			AsymmetricPublicKey() {}
			virtual ~AsymmetricPublicKey() {}
			virtual bool verify(const std::string &text, const std::string &signature)=0;
			virtual std::string &serialize(std::string &buffer)=0;
		private:
			AsymmetricPublicKey(const AsymmetricPublicKey&);
			AsymmetricPublicKey &operator=(const AsymmetricPublicKey&);
	};

	class AsymmetricPrivateKey {
		public:
			AsymmetricPrivateKey() {}
			virtual ~AsymmetricPrivateKey() {}
			virtual AsymmetricPublicKey *publicKey()=0;
			virtual std::string &serialize(std::string &buffer)=0;
			virtual std::string &sign(const std::string &text, std::string &signature)=0;
		private:
			AsymmetricPrivateKey(const AsymmetricPrivateKey&);
			AsymmetricPrivateKey &operator=(const AsymmetricPrivateKey&);
	};

	template<typename T>
	class AutoClean {
		public:
			AutoClean(T *p=nullptr):data(p) {}
			~AutoClean() {dispose();}
			operator const T*() const {return data;}
			operator T*() const {return data;}
			T* operator->() const {return data;}
			AutoClean<T> &operator=(const AutoClean<T> &other) {data = other.data; return *this;}
			void dispose() {if (nullptr != data) {delete data; data = nullptr;}}
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

#if OpenSSLAvailable

	class OpenSSLRSA {
		public:
			OpenSSLRSA(const int keySize, const unsigned long publicExponent = RSA_F4) // publicExponent = RSA_3
					:_rsa(__crypto_OSSLHandle(RSA_new())) {
				AutoClean<BIGNUM> bigPublicExponent(__crypto_OSSLHandle(BN_new()));

				__crypto_OSSLHandle(BN_set_word(bigPublicExponent, publicExponent));
				__crypto_OSSLHandle(RSA_generate_key_ex(_rsa.data, keySize, bigPublicExponent, nullptr));
			}
			typedef RSA*(*KeyReader)(BIO*,RSA**,pem_password_cb*,void*); // PEM_read_bio_RSAPublicKey or PEM_read_bio_RSAPrivateKey
			OpenSSLRSA(const std::string &serialized, KeyReader keyReader):_rsa(nullptr) {
				AutoClean<BIO> memory(__crypto_OSSLHandle(BIO_new_mem_buf(serialized.data(), serialized.size())));

				_rsa.data = __crypto_OSSLHandle(keyReader(memory, nullptr, nullptr, nullptr));
			}
			std::string &serializePrivate(std::string &buffer) {
				serializeKey(buffer, _writePrivate);
				return buffer;
			}
			std::string &serializePublic(std::string &buffer) {
				serializeKey(buffer, _writePublic);
				return buffer;
			}
			typedef const EVP_MD*(*MessageDigestType)(void);
			std::string &sign(const std::string &text, std::string &signature, MessageDigestType messageDigestType) {  // EVP_sha256
				AutoClean<EVP_MD_CTX> signer(__crypto_OSSLHandle(EVP_MD_CTX_create()));
				AutoClean<EVP_PKEY> key(__crypto_OSSLHandle(EVP_PKEY_new()));
				size_t signatureSize = 0;

				__crypto_OSSLHandle(EVP_PKEY_set1_RSA(key, _rsa));
				__crypto_OSSLHandle(EVP_DigestSignInit(signer, nullptr, messageDigestType(), nullptr, key));
				__crypto_OSSLHandle(EVP_DigestSignUpdate(signer, text.data(), text.size()));
				__crypto_OSSLHandle(EVP_DigestSignFinal(signer, nullptr, &signatureSize));
				signature.assign(signatureSize, '\0');
				__crypto_OSSLHandle(EVP_DigestSignFinal(signer, reinterpret_cast<unsigned char*>(const_cast<char*>(signature.data())), &signatureSize));
				return signature; // binary signature
			}
			bool verify(const std::string &text, const std::string &signature, MessageDigestType messageDigestType) {  // EVP_sha256
				AutoClean<EVP_MD_CTX> verifier = (__crypto_OSSLHandle(EVP_MD_CTX_create()));
				AutoClean<EVP_PKEY> key(__crypto_OSSLHandle(EVP_PKEY_new()));
				int status = -1;

				__crypto_OSSLHandle(EVP_PKEY_set1_RSA(key, _rsa));
				__crypto_OSSLHandle(EVP_DigestVerifyInit(verifier, nullptr, messageDigestType(), nullptr, key));
				__crypto_OSSLHandle(EVP_DigestVerifyUpdate(verifier, text.data(), text.size()));
				status = EVP_DigestVerifyFinal(verifier, reinterpret_cast<const unsigned char *>(signature.data()), signature.size());
				if (status == 1) {
					return true;
				}
				if (status != 0) {
					handleOpenSSLResult(0, "EVP_DigestVerifyFinal", __FILE__, __LINE__); // not tested
				}
				return false;
			}
			~OpenSSLRSA() {
			}
		private:
			AutoClean<RSA> _rsa;
			typedef int(*KeyTypeSerializer)(BIO *b, RSA *r);
			static int _writePrivate(BIO *b, RSA *r) {
				return PEM_write_bio_RSAPrivateKey(b, r, nullptr, nullptr, 0, nullptr, nullptr);
			}
			static int _writePublic(BIO *b, RSA *r) {
				return PEM_write_bio_RSAPublicKey(b, r);
			}
			void serializeKey(std::string &buffer, KeyTypeSerializer keytypeSerializer) {
				AutoClean<BIO> memory(__crypto_OSSLHandle(BIO_new(BIO_s_mem())));
				__crypto_OSSLHandle(keytypeSerializer(memory, _rsa));

				const int dataSize = BIO_pending(memory);

				buffer.assign(dataSize, '\0');
				__crypto_OSSLHandle(BIO_read(memory, const_cast<char*>(buffer.data()), dataSize));
			}
			OpenSSLRSA(const OpenSSLRSA&);
			OpenSSLRSA &operator=(const OpenSSLRSA&);
	};

	class OpenSSLRSAAES256PublicKey : public AsymmetricPublicKey {
		public:
			OpenSSLRSAAES256PublicKey(const std::string &serialized)
				:rsa(serialized, PEM_read_bio_RSAPublicKey) {}
			virtual ~OpenSSLRSAAES256PublicKey() {}
			virtual std::string &serialize( std::string &buffer) {
				return rsa.serializePublic(buffer);
			}
			virtual bool verify(const std::string &text, const std::string &signature) {
				return rsa.verify(text, signature, EVP_sha256);
			}
		private:
			OpenSSLRSA rsa;
			OpenSSLRSAAES256PublicKey(const OpenSSLRSAAES256PublicKey&);
			OpenSSLRSAAES256PublicKey &operator=(const OpenSSLRSAAES256PublicKey&);
	};

	class OpenSSLRSAAES256PrivateKey : public AsymmetricPrivateKey {
		public:
			OpenSSLRSAAES256PrivateKey(const int keySize, const unsigned long publicExponent=3)
				:rsa(keySize, publicExponent) {}
			OpenSSLRSAAES256PrivateKey(const std::string &serialized)
				:rsa(serialized, PEM_read_bio_RSAPrivateKey) {}
			virtual ~OpenSSLRSAAES256PrivateKey() {}
			virtual std::string &serialize(std::string &buffer) {
				return rsa.serializePrivate(buffer);
			}
			virtual std::string &sign(const std::string &text, std::string &signature) {
				return rsa.sign(text, signature, EVP_sha256);
			}
			virtual AsymmetricPublicKey *publicKey() {
				std::string buffer;

				return new OpenSSLRSAAES256PublicKey(rsa.serializePublic(buffer));
			}
		private:
			OpenSSLRSA rsa;
			OpenSSLRSAAES256PrivateKey(const OpenSSLRSAAES256PrivateKey&);
			OpenSSLRSAAES256PrivateKey &operator=(const OpenSSLRSAAES256PrivateKey&);
	};

#endif // OpenSSLAvailable

}

#endif // __AsymmetricEncrypt_h__
