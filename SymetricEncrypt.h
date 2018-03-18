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

#include <CommonCrypto/CommonCryptor.h>

#define CCHandle(call) crypto::Exception::handle( (call), #call, __FILE__, __LINE__)

namespace crypto {

	class Exception : public msg::Exception {
		public:
			static void handle(CCCryptorStatus status, const std::string &call, const char *file= NULL, int line= 0);
			/// Generic crypto exception
			Exception(const std::string &message, const char *file= NULL, int line= 0) throw():msg::Exception(message, file, line) {}
			/// destructs _message
			virtual ~Exception() throw() {}
	};

	#define DeclareError(name, message) \
	class name##Error : public Exception { \
		public: \
			name##Error(const std::string &call, const char *file= NULL, int line= 0) throw():Exception(call + " : " message, file, line) {} \
			virtual ~name##Error() throw() {} \
	}

	DeclareError(Param, "Illegal parameter value");
	DeclareError(BufferTooSmall, "Insufficient buffer provided for specified operation");
	DeclareError(Memory, "Memory allocation failure");
	DeclareError(Alignment, "Input size was not aligned properly");
	DeclareError(Decode, "Input data did not decode or decrypt properly");
	DeclareError(Unimplemented, "Function not implemented for the current algorithm");
	DeclareError(IVWrongSize, "Function not implemented for the current algorithm");

	void Exception::handle(CCCryptorStatus status, const std::string &call, const char *file, int line) {
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
	class SymetricCryptor {
		public:
			SymetricCryptor(const void *key):_key(reinterpret_cast<const char*>(key), keyLength) {}
			SymetricCryptor(const std::string &key):_key(key) {
				// TODO throw exception if _key.length() != keyLength
			}
			virtual ~SymetricCryptor() {}
			size_t key_length() {return keyLength;}
			size_t iv_length() {return ivLength;}
			size_t encrypt(const void *data, size_t length, void *out, size_t outSize, const void *iv=NULL) {
				return _crypt(kCCEncrypt, data, length, out, outSize, iv);
			}
			size_t decrypt(const void *data, size_t length, void *out, size_t outSize, const void *iv=NULL) {
				return _crypt(kCCDecrypt, data, length, out, outSize, iv);
			}
			std::string &encrypt(const std::string &data, std::string &out, const std::string &iv="") {
				return _crypt(kCCEncrypt, data, out, iv);
			}
			std::string &decrypt(const std::string &data, std::string &out, const std::string &iv="") {
				return _crypt(kCCDecrypt, data, out, iv);
			}
			std::string encrypt(const std::string &data, const std::string &iv="") {
				std::string out;

				return _crypt(kCCEncrypt, data, out, iv);
			}
			std::string decrypt(const std::string &data, const std::string &iv="") {
				std::string out;

				return _crypt(kCCDecrypt, data, out, iv);
			}
		private:
			std::string _key;
			size_t _crypt(CCOperation op, const void *data, size_t length, void *out, size_t outSize, const void *iv) {
				size_t outDataSize = 0;

				CCHandle(CCCrypt(op, algorithm, options, _key.data(), _key.length(), iv, data, length, out, outSize, &outDataSize));
				return outDataSize;
			}
			std::string &_crypt(CCOperation op, const std::string &data, std::string &out, const std::string &iv) {
				size_t	outSize;

				if ( (iv.length() != 0) && (iv.length() != ivLength) ) {
					throw IVWrongSizeError("Initialization Vector (iv) is wrong size", __FILE__, __LINE__);
				}
				out.assign(data.length() + blockSize, '\0');
				outSize = _crypt(op, data.data(), data.length(), const_cast<char*>(out.data()), out.length(), iv.data());
				out.erase(outSize);
				return out;
			}
	};

	typedef SymetricCryptor<kCCAlgorithmAES, kCCOptionPKCS7Padding, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> AES256_CBC_Padded;
	typedef SymetricCryptor<kCCAlgorithmAES, 0, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> AES256_CBC;
	typedef SymetricCryptor<kCCAlgorithmAES, kCCOptionECBMode | kCCOptionPKCS7Padding, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> AES256_EBC_Padded;
	typedef SymetricCryptor<kCCAlgorithmAES, kCCOptionECBMode, kCCKeySizeAES256, kCCBlockSizeAES128, kCCBlockSizeAES128> AES256_EBC;
	typedef AES256_CBC_Padded AES256;

}

#undef CCHandle
#undef DeclareError

#endif // __SymetricEncrypt_h__
