#ifndef __AsymmetricEncrypt_h__
#define __AsymmetricEncrypt_h__

/** @file AsymmetricEncrypt.h
	@todo document
	@todo genericize like Hash to move Mac specific code into platform isolcation
*/

#include "CryptoHelpers.h"

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

	typedef void(*Cleanup)(void*);
	template<typename T, Cleanup cleanup>
	class AutoClean {
		public:
			AutoClean(T *p):data(p) {}
			~AutoClean() {if (nullptr != data) {cleanup(data);}}
			operator const T*() const {return data;}
			operator T*() const {return data;}
	};

	class OpenSSLRSA {
		public:
			OpenSSLRSA(const int keySize, const unsigned long publicExponent = RSA_F4) {
				AutoClean<RSA,RSA_free> rsa(__crypto_OSSLHandle(RSA_new()));
				AutoClean<BIGNUM, BN_free> bigPublicExponent(__crypto_OSSLHandle(BN_new()));

				__crypto_OSSLHandle(BN_set_word(bigPublicExport, publicExponent));
				__crypto_OSSLHandle(RSA_generate_key_ex(rsa, keySize, bigPublicExponent, nullptr));
				_key = rsa.data;
				rsa.data = nullptr;
			}
			void serializePrivate(std::string &buffer) {
				serializeKey<_writePrivate>(buffer);
			}
			void serializePublic(std::string &buffer) {
				serializeKey<PEM_write_bio_RSAPublicKey>(buffer);
			}
			~OpenSSLRSA() {
				RSA_free(_key);
			}
		private:
			RSA *_key;
			typedef int(*KeyTypeSerializer)(BIO *b, RSA *r);
			static int _writePrivate(BIO *b, RSA *r) {
				return PEM_write_bio_RSAPrivateKey(b, r, nullptr, nullptr, 0, nullptr, nullptr);
			}
			template<KeyTypeSerializer keytypeSerializer>
			void serializeKey(std::string &buffer) {
				AutoClean<BIO, BIO_free_all> memory(__crypto_OSSLHandle(BIO_new(BIO_s_mem())));
				__crypto_OSSLHandle(keytypeSerializer(memory, _key));
				buffer.clear();

				const int dataSize = BIO_pending(memory);

				buffer.assign(dataSize, '\0');
				BIO_read(memory, const_cast<char*>(buffer.data()), dataSize);
			}
	};


}

#endif // __AsymmetricEncrypt_h__

/*
 int PEM_write_bio_RSAPrivateKey(BIO *bp, RSA *x, const EVP_CIPHER *enc,
                                 unsigned char *kstr, int klen,
                                 pem_password_cb *cb, void *u);

 int PEM_write_bio_RSAPublicKey(BIO *bp, RSA *x);

bool generate_key()
{
	int				ret = 0;
	RSA				*r = NULL;
	BIGNUM			*bne = NULL;
	BIO				*bp_public = NULL, *bp_private = NULL;

	int				bits = 2048;
	unsigned long	e = RSA_F4;

	// 1. generate rsa key
	bne = BN_new();
	ret = BN_set_word(bne,e);
	if(ret != 1){
		goto free_all;
	}

	r = RSA_new();
	ret = RSA_generate_key_ex(r, bits, bne, NULL);
	if(ret != 1){
		goto free_all;
	}

	// 2. save public key
	bp_public = BIO_new_file("public.pem", "w+");
	ret = PEM_write_bio_RSAPublicKey(bp_public, r);
	if(ret != 1){
		goto free_all;
	}

	// 3. save private key
	bp_private = BIO_new_file("private.pem", "w+");
	ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

	// 4. free
free_all:

	BIO_free_all(bp_public);
	BIO_free_all(bp_private);
	RSA_free(r);
	BN_free(bne);

	return (ret == 1);
}
*/