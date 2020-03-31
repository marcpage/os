#ifndef __AsymmetricEncrypt_h__
#define __AsymmetricEncrypt_h__

/** @file AsymmetricEncrypt.h
	@todo document
	@todo genericize like Hash to move Mac specific code into platform isolcation
*/

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

}

#endif // __AsymmetricEncrypt_h__
