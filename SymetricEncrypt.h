#ifndef __SymetricEncrypt_h__
#define __SymetricEncrypt_h__

/** @file SymetricEncrypt.h
	@todo document
	@todo genericize like Hash to move Mac specific code into platform isolcation
*/

#include <string>
#include <string.h>
#include <ctype.h>
#include "Exception.h"

#if __APPLE_CC__ || __APPLE__
	#include <CommonCrypto/CommonCryptor.h>
#endif

#if OpenSSLAvailable
	#include <openssl/evp.h>
	#include <openssl/aes.h>
	#include <openssl/err.h>
#endif

namespace crypto {

	class SymetricKey {
		public:
			SymetricKey() {}
			virtual ~SymetricKey() {}
			std::string encrypt(const std::string &data) const;
			std::string decrypt(const std::string &encrypted) const;
			std::string encryptWithIV(const std::string &data, const std::string &iv) const;
			std::string decryptWithIV(const std::string &encrypted, const std::string &iv) const;
			virtual std::string &encryptInPlace(const std::string &data, const std::string &iv, std::string &encrypted) const=0;
			virtual std::string &decryptInPlace(const std::string &encrypted, const std::string &iv, std::string &data) const=0;
	};

	template<class SpecificCryptor>
	class SpecificSymetricKey : public SymetricKey {
		public:
			SpecificSymetricKey(const void *data, size_t dataSize);
			SpecificSymetricKey(const std::string &data);
			virtual ~SpecificSymetricKey() {}
			virtual std::string &encryptInPlace(const std::string &data, const std::string &iv, std::string &encrypted) const;
			virtual std::string &decryptInPlace(const std::string &encrypted, const std::string &iv, std::string &data) const;
		private:
			std::string _key;
	};

	class Exception : public msg::Exception {
		public:
			/// Generic crypto exception
			Exception(const std::string &message, const char *file= NULL, int line= 0) throw():msg::Exception(message, file, line) {}
			/// destructs _message
			virtual ~Exception() throw() {}
	};

	#define EncryptAssert(name, condition) if (!(condition)) {throw name##Error(#condition, __FILE__, __LINE__);} else msg::noop()
	#define DeclareError(name, message) \
	class name##Error : public Exception { \
		public: \
			name##Error(const std::string &call, const char *file= NULL, int line= 0) throw():Exception(call + " : " message, file, line) {} \
			virtual ~name##Error() throw() {} \
	}

	DeclareError(KeySize, "Key data is incorrect size");
	DeclareError(Param, "Illegal parameter value");
	DeclareError(BufferTooSmall, "Insufficient buffer provided for specified operation");
	DeclareError(Memory, "Memory allocation failure");
	DeclareError(Alignment, "Input size was not aligned properly");
	DeclareError(Decode, "Input data did not decode or decrypt properly");
	DeclareError(Unimplemented, "Function not implemented for the current algorithm");
	DeclareError(IVWrongSize, "Initialization Vector is the wrong size");

	inline std::string SymetricKey::encrypt(const std::string &data) const {
		std::string encrypted;

		return encryptInPlace(data, "", encrypted);
	}
	inline std::string SymetricKey::decrypt(const std::string &encrypted) const {
		std::string decrypted;

		return decryptInPlace(encrypted, "", decrypted);
	}
	inline std::string SymetricKey::encryptWithIV(const std::string &data, const std::string &iv) const {
		std::string encrypted;

		return encryptInPlace(data, iv, encrypted);
	}
	inline std::string SymetricKey::decryptWithIV(const std::string &encrypted, const std::string &iv) const {
		std::string decrypted;

		return decryptInPlace(encrypted, iv, decrypted);
	}

	template<class SpecificCryptor>
	inline SpecificSymetricKey<SpecificCryptor>::SpecificSymetricKey(const void *data, size_t dataSize):_key(reinterpret_cast<const char*>(data), dataSize) {
		EncryptAssert(KeySize, dataSize == SpecificCryptor::Size);
	}
	template<class SpecificCryptor>
	inline SpecificSymetricKey<SpecificCryptor>::SpecificSymetricKey(const std::string &data):_key(data) {
		EncryptAssert(KeySize, data.length() == SpecificCryptor::Size);
	}
	template<class SpecificCryptor>
	inline std::string &SpecificSymetricKey<SpecificCryptor>::encryptInPlace(const std::string &data, const std::string &iv, std::string &encrypted) const {
		EncryptAssert(IVWrongSize, (iv.length() == SpecificCryptor::IVLength) || (iv.length() == 0));
		encrypted.assign(data.size() + SpecificCryptor::BlockSize, '\0');

		size_t encryptedSize= SpecificCryptor::encrypt(_key.data(), data.data(), data.length(), const_cast<char*>(encrypted.data()), encrypted.length(), iv.length() ? iv.data() : NULL);

		encrypted.erase(encryptedSize);
		return encrypted;
	}
	template<class SpecificCryptor>
	inline std::string &SpecificSymetricKey<SpecificCryptor>::decryptInPlace(const std::string &encrypted, const std::string &iv, std::string &data) const {
		EncryptAssert(IVWrongSize, (iv.length() == SpecificCryptor::IVLength) || (iv.length() == 0));
		data.assign(encrypted.size() + SpecificCryptor::BlockSize, '\0');

		size_t encryptedSize= SpecificCryptor::decrypt(_key.data(), encrypted.data(), encrypted.length(), const_cast<char*>(data.data()), data.length(), iv.length() ? iv.data() : NULL);

		data.erase(encryptedSize);
		return data;
	}

#if OpenSSLAvailable

	#define OSSLHandle(call) handleOpenSSLResult( (call), #call, __FILE__, __LINE__)

	void handleOpenSSLResult(int status, const std::string &call, const char *file, int line) {
		if(!status) {
			std::string buffer(512, '\0');
			ERR_error_string(ERR_get_error(), const_cast<char*>(buffer.data()));
			buffer.erase(strlen(buffer.c_str()));
			throw Exception(std::string("OpenSSL Error (" + call + "): ") + buffer, file, line);
		}
	}

	void handleOpenSSLResult(void *pointer, const std::string &call, const char *file, int line) {
		handleOpenSSLResult(NULL == pointer ? 0 : 1, call, file, line);
	}

	class OpenSSLContext {
		public:
			OpenSSLContext() {
				OSSLHandle(_context = EVP_CIPHER_CTX_new());
			}
			~OpenSSLContext() {
				OSSLHandle(EVP_CIPHER_CTX_cleanup(_context));
			}
			operator const EVP_CIPHER_CTX*() const {
				return _context;
			}
			operator EVP_CIPHER_CTX*() const {
				return _context;
			}
		private:
			EVP_CIPHER_CTX *_context;
	};

	template<typename cipher, bool padding, int keySize, int blockSize, int ivSize>
	struct OpenSSLAES {
		enum {
			Size= keySize
		};
		enum {
			BlockSize= blockSize
		};
		enum {
			IVLength= ivSize
		};
		static size_t encrypt(const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			return _crypt(1, key, data, length, out, outBufferSize, iv);
		}
		static size_t decrypt(const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			return _crypt(0, key, data, length, out, outBufferSize, iv);
		}
		private:
		static size_t _crypt(int enc, const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			unsigned char *outBuffer = reinterpret_cast<unsigned char*>(out);
			int	bytesWritten;
			OpenSSLContext context;
			const EVP_CIPHER *cipher_type = cipher();

			OSSLHandle(EVP_CipherInit_ex(context, cipher_type, NULL, reinterpret_cast<const unsigned char*>(key), reinterpret_cast<const unsigned char*>(iv), enc));
			AssertMessageException(EVP_CIPHER_CTX_key_length(context) == Size);
			AssertMessageException( (NULL == iv) || (EVP_CIPHER_CTX_iv_length(context) == IVLength) );
			OSSLHandle(EVP_CIPHER_CTX_set_padding(context, padding ? 1 : 0));

			OSSLHandle(EVP_CipherUpdate(context, outBuffer, &bytesWritten, reinterpret_cast<const unsigned char*>(data), length));
			OSSLHandle(EVP_CipherFinal_ex(context, outBuffer, &bytesWritten));
			return bytesWritten;
		}
	};

	typedef OpenSSLAES<EVP_aes_256_cbc> OpenSSL_AES256_CBC_Padded_Cryptor;
	typedef OpenSSLAES<EVP_aes_256_cbc> OpenSSL_AES256_CBC_Cryptor;

EVP_aes_256_cbc (void)
const EVP_CIPHER * 	EVP_aes_128_cfb8 (void)
const EVP_CIPHER * 	EVP_aes_192_cfb8 (void)
const EVP_CIPHER * 	EVP_aes_256_cfb8 (void)


#if __APPLE_CC__ || __APPLE__
	typedef SpecificSymetricKey<OpenSSL_AES256_CBC_Padded_Cryptor, true, 32, AES_BLOCK_SIZE, AES_BLOCK_SIZE> OpenSSL_AES256_CBC_Padded;
	typedef SpecificSymetricKey<OpenSSL_AES256_CBC_Cryptor, false, 32, AES_BLOCK_SIZE, AES_BLOCK_SIZE> OpenSSL_AES256_CBC;

	typedef OpenSSL_AES256_CBC_Padded OpenSSL_AES256;
#else
	typedef SpecificSymetricKey<OpenSSL_AES256_CBC_Padded_Cryptor> AES256_CBC_Padded;
	typedef SpecificSymetricKey<OpenSSL_AES256_CBC_Cryptor> AES256_CBC;

	typedef AES256_CBC_Padded AES256;
#endif

	#undef OSSLHandle
#endif

#if __APPLE_CC__ || __APPLE__
	#define CCHandle(call) handleCCCryptorStatus( (call), #call, __FILE__, __LINE__)

	void handleCCCryptorStatus(CCCryptorStatus status, const std::string &call, const char *file, int line) {
		switch(status) {
     		case kCCSuccess:
     			break;
     		case kCCParamError:
     			throw ParamError(call, file, line);
     		case kCCBufferTooSmall:
     			throw BufferTooSmallError(call, file, line);
     		case kCCMemoryFailure:
     			throw MemoryError(call, file, line);
     		case kCCAlignmentError:
     			throw AlignmentError(call, file, line);
     		case kCCDecodeError:
     			throw DecodeError(call, file, line);
     		case kCCUnimplemented:
     			throw UnimplementedError(call, file, line);
     		default:
     			throw new Exception(call, file, line);
     	}
	}

	template<CCAlgorithm algorithm, CCOptions options, size_t keyLength, size_t blockSize, size_t ivLength>
	struct CommonCryptoKey {
		enum {
			Size= keyLength
		};
		enum {
			BlockSize= blockSize
		};
		enum {
			IVLength= ivLength
		};
		static size_t encrypt(const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			return _crypt(kCCEncrypt, key, data, length, out, outBufferSize, iv);
		}
		static size_t decrypt(const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			return _crypt(kCCDecrypt, key, data, length, out, outBufferSize, iv);
		}
		private:
		static size_t _crypt(CCOperation op, const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			size_t outDataSize = 0;

			CCHandle(CCCrypt(op, algorithm, options, key, keyLength, iv, data, length, out, outBufferSize, &outDataSize));
			return outDataSize;
		}
	};

	typedef CommonCryptoKey<kCCAlgorithmAES, kCCOptionPKCS7Padding, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> CommonCrypto_AES256_CBC_Padded_Cryptor;
	typedef CommonCryptoKey<kCCAlgorithmAES, 0, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> CommonCrypto_AES256_CBC_Cryptor;
	typedef CommonCryptoKey<kCCAlgorithmAES, kCCOptionECBMode | kCCOptionPKCS7Padding, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> CommonCrypto_AES256_EBC_Padded_Cryptor;
	typedef CommonCryptoKey<kCCAlgorithmAES, kCCOptionECBMode, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> CommonCrypto_AES256_EBC_Cryptor;

	typedef SpecificSymetricKey<CommonCrypto_AES256_CBC_Padded_Cryptor> AES256_CBC_Padded;
	typedef SpecificSymetricKey<CommonCrypto_AES256_CBC_Cryptor> AES256_CBC;
	typedef SpecificSymetricKey<CommonCrypto_AES256_EBC_Padded_Cryptor> AES256_EBC_Padded;
	typedef SpecificSymetricKey<CommonCrypto_AES256_EBC_Cryptor> AES256_EBC;

	typedef AES256_CBC_Padded AES256;

	// Cleanup internal define
	#undef CCHandle
#endif
}

#undef EncryptAssert
#undef DeclareError

#endif // __SymetricEncrypt_h__
