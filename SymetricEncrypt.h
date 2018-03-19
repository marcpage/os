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

namespace crypto {

	class SymetricKey {
		public:
			SymetricKey() {}
			virtual ~SymetricKey() {}
			std::string encrypt(const std::string &data) const;
			std::string decrypt(const std::string &encrypted) const;
			std::string encrypt(const std::string &data, const std::string &iv) const;
			std::string decrypt(const std::string &encrypted, const std::string &iv) const;
			virtual std::string &encrypt(const std::string &data, const std::string &iv, std::string &encrypted) const=0;
			virtual std::string &decrypt(const std::string &encrypted, const std::string &iv, std::string &data) const=0;
	};

	template<class SpecificCryptor>
	class SpecificSymetricKey : public SymetricKey {
		public:
			SpecificSymetricKey(const void *data, size_t dataSize);
			SpecificSymetricKey(const std::string &data);
			virtual ~SpecificSymetricKey() {}
			virtual std::string &encrypt(const std::string &data, const std::string &iv, std::string &encrypted) const;
			virtual std::string &decrypt(const std::string &encrypted, const std::string &iv, std::string &data) const;
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

	#define EncryptAssert(name, condition) if (!(condition)) {throw name##Error(condition, __FILE__, __LINE__);} else msg::noop()
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
	DeclareError(IVWrongSize, "Function not implemented for the current algorithm");

	inline std::string SymetricKey::encrypt(const std::string &data) const {
		std::string encrypted;

		return encrypt(data, "", encrypted);
	}
	inline std::string SymetricKey::decrypt(const std::string &encrypted) const {
		std::string decrypted;

		return encrypt(encrypted, "", decrypted);
	}
	inline std::string SymetricKey::encrypt(const std::string &data, const std::string &iv) const {
		std::string encrypted;

		return encrypt(data, iv, encrypted);
	}
	inline std::string SymetricKey::decrypt(const std::string &encrypted, const std::string &iv) const {
		std::string decrypted;

		return encrypt(encrypted, iv, decrypted);
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
	inline std::string &SpecificSymetricKey<SpecificCryptor>::encrypt(const std::string &data, const std::string &iv, std::string &encrypted) const {
		EncryptAssert(IVWrongSize, iv.length() == SpecificCryptor::IVLength);
		return SpecificCryptor::encrypt(data, iv, encrypted);
	}
	template<class SpecificCryptor>
	inline std::string &SpecificSymetricKey<SpecificCryptor>::decrypt(const std::string &encrypted, const std::string &iv, std::string &data) const {
		EncryptAssert(IVWrongSize, iv.length() == SpecificCryptor::IVLength);
		return SpecificCryptor::decrypt(data, iv, encrypted);
	}

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
			size_t outDataSize = 0;

			CCHandle(CCCrypt(kCCEncrypt, algorithm, options, key, keyLength, iv, data, length, out, outBufferSize, &outDataSize));
			return outDataSize;
		}
		static size_t decrypt(const void *key, const void *data, size_t length, void *out, size_t outBufferSize, const void *iv) {
			size_t outDataSize = 0;

			CCHandle(CCCrypt(kCCDecrypt, algorithm, options, key, keyLength, iv, data, length, out, outBufferSize, &outDataSize));
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
	#undef EncryptAssert
	#undef DeclareError
#endif
}

#undef CCHandle
#undef DeclareError

#endif // __SymetricEncrypt_h__
