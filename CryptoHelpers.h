#ifndef __OpenSLLHelpers_h__
#define __OpenSLLHelpers_h__

#include "os/Exception.h"

#if OpenSSLAvailable
	#include <openssl/err.h>
	#include <openssl/evp.h>
#endif

#if __APPLE_CC__ || __APPLE__
	#include <CommonCrypto/CommonCryptor.h>
#endif

namespace crypto {

	class CryptoException : public msg::Exception {
		public:
			/// Generic crypto exception
			explicit CryptoException(const std::string &message, const char *file= NULL, int line= 0) throw():msg::Exception(message, file, line) {}
			/// destructs _message
			virtual ~CryptoException() throw() {}
	};

	#define __crypto_EncryptAssert(name, condition) if (!(condition)) {throw name##Error(#condition, __FILE__, __LINE__);} else msg::noop()
	#define DeclareError(name, message) \
	class name##Error : public CryptoException { \
		public: \
			explicit name##Error(const std::string &call, const char *file= NULL, int line= 0) throw():CryptoException(call + " : " message, file, line) {} \
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

	#undef DeclareError

#if __APPLE_CC__ || __APPLE__

	#define __crypto_CCHandle(call) handleCCCryptorStatus( (call), #call, __FILE__, __LINE__)

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
     			throw new CryptoException(call, file, line);
     	}
	}

#endif // __APPLE_CC__ || __APPLE__

#if OpenSSLAvailable

	#define __crypto_OSSLHandle(call) handleOpenSSLResult( (call), #call, __FILE__, __LINE__)

	int handleOpenSSLResult(int status, const std::string &call, const char *file, int line) {
		if(!status) {
			std::string buffer(512, '\0');
			ERR_error_string(ERR_get_error(), const_cast<char*>(buffer.data()));
			buffer.erase(strlen(buffer.c_str()));
			throw CryptoException(std::string("OpenSSL Error (" + call + "): ") + buffer, file, line);
		}
		return status;
	}

	template<typename T>
	T *handleOpenSSLResult(T *pointer, const std::string &call, const char *file, int line) {
		handleOpenSSLResult(NULL == pointer ? 0 : 1, call, file, line);
		return pointer;
	}

	class OpenSSLContext {
		public:
			OpenSSLContext() {
				__crypto_OSSLHandle(_context = EVP_CIPHER_CTX_new());
			}
			~OpenSSLContext() {
				__crypto_OSSLHandle(EVP_CIPHER_CTX_cleanup(_context));
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

#endif //  OpenSSLAvailable

}


#endif // __OpenSLLHelpers_h__
