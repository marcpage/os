
// clang++ 2clean/AsymetricEncrypt.cpp -framework Security -framework CoreFoundation -DOpenSSLAvailable=1 -I.. -I2Clean -I/usr/local/Cellar/openssl@1.1/1.1.1d/include -L/usr/local/Cellar/openssl@1.1/1.1.1d/lib -lcrypto -o /tmp/test

#include <stdio.h>
#include <string>


/*
	Asymmetric Encrypt Behaviors

		Public Key
			1. Verify Signature
			2. Serialize
			3. Deserialize

		Private Key
			1. Sign
			2. Serialize
			3. Deserialize
			4. Get public key
			5. Generate

*/


// Recommended public exponents: 3, 5, 17, 257 or 65537

#define AppleAPI 0

#if AppleAPI

#if __APPLE_CC__ || __APPLE__
	#include <CommonCrypto/CommonCryptor.h>
	#include <Security/Security.h>
#endif

std::string dumpKey(const void *key) {
	OSStatus res;
	CFDataRef exported;

	if (!key) {
		printf("key is null\n");
		return "";
	}
	res = SecItemExport(key, kSecFormatUnknown, kSecItemPemArmour, nullptr, &exported);
	if (!exported) {
		printf("Unable to export key data\n");
		return "";
	}
	CFIndex size = CFDataGetLength(exported);

	std::string value(std::string::size_type(size), '\0');
	CFDataGetBytes(exported, CFRangeMake(0,size), reinterpret_cast<UInt8*>(const_cast<char*>(value.data())));
	printf("value = '%s'\n", value.c_str());
	return value;
}

int main(int argc, char* argv[]) {
	SecKeyRef publicKey, privateKey;
	OSStatus res;
	int keySize = (argc == 2) ? atoi(argv[1]) : 1024; // 1024 - 4096 increment 8

	/* Time to generate key
		1024 = 0.25 seconds
		2048 - 2 seconds
		3072 - 8 seconds
		4096 - 30 seconds
	*/

    SecItemImportExportKeyParameters params;
    SecExternalItemType itemType = kSecItemTypeUnknown;
    SecExternalFormat format = kSecFormatUnknown;

	params.flags = kSecKeyNoAccessControl;
	params.passphrase = nullptr;

	CFMutableDictionaryRef attrDict = CFDictionaryCreateMutable(nullptr, 3, &kCFTypeDictionaryKeyCallBacks, nullptr);
	CFDictionaryAddValue(attrDict, kSecAttrKeyType, kSecAttrKeyTypeRSA);
	CFDictionaryAddValue(attrDict, kSecAttrKeySizeInBits, CFNumberCreate(nullptr, kCFNumberIntType, &keySize));

	res = SecKeyGeneratePair(attrDict, &publicKey, &privateKey);

	const void *elements[] = {publicKey, privateKey};
	CFArrayRef keys = CFArrayCreate(nullptr, elements, 2, &kCFTypeArrayCallBacks);
	std::string keyText = dumpKey(keys);
	std::string pub = dumpKey(publicKey);
	std::string priv = dumpKey(privateKey);

	CFArrayRef keyList;
	CFDataRef data;

	data = CFDataCreate(nullptr, reinterpret_cast<const UInt8 *>(pub.data()), pub.size());
	res = SecItemImport(data, nullptr, &format, &itemType, 0, &params, nullptr, &keyList);
	printf("res=%d\n", res);
	printf("length=%ld\n", (long)CFArrayGetCount(keyList));

	SecKeyRef pubCopy = reinterpret_cast<SecKeyRef>(const_cast<void*>(CFArrayGetValueAtIndex(keyList, 0)));
	printf("pubCopy = %p\n", pubCopy);
	dumpKey(pubCopy);

	data = CFDataCreate(nullptr, reinterpret_cast<const UInt8 *>(priv.data()), priv.size());
	res = SecItemImport(data, nullptr, &format, &itemType, 0, &params, nullptr, &keyList);
	printf("res=%d\n", res);
	printf("length=%ld\n", (long)CFArrayGetCount(keyList));
	printf("Keysize = %d\n", keySize);
	// SecKeyCreateEncryptedData https://developer.apple.com/documentation/security/1643957-seckeycreateencrypteddata?language=objc

	SecKeyRef privCopy = reinterpret_cast<SecKeyRef>(const_cast<void*>(CFArrayGetValueAtIndex(keyList, 0)));
	printf("privCopy = %p\n", privCopy);
	dumpKey(privCopy);

	return 0;
}

#elif 1

#include "AsymmetricEncrypt.h"
int main(int argc, char* argv[]) {
	crypto::OpenSSLRSA	rsa(1024);
	std::string signature;

	std::string publicKey,privateKey;
	rsa.serializePublic(publicKey); // crash EXC_BAD_ACCESS (code=1, address=0x80280ec0)
	rsa.serializePrivate(privateKey);
	printf("public\n%s\n", publicKey.c_str());
	printf("private\n%s\n", privateKey.c_str());

	crypto::OpenSSLRSA	publicRsa(publicKey, PEM_read_bio_RSAPublicKey);
	crypto::OpenSSLRSA	privateRsa(privateKey, PEM_read_bio_RSAPrivateKey);

	publicKey.clear();
	privateKey.clear();

	publicRsa.serializePublic(publicKey);
	privateRsa.serializePrivate(privateKey);
	printf("public\n%s\n", publicKey.c_str());
	printf("private\n%s\n", privateKey.c_str());

	privateRsa.sign("testing", signature, EVP_sha256);
	if (!publicRsa.verify("testing", signature, EVP_sha256)) {
		printf("Failed to verify 2\n");
	}

	rsa.sign("testing", signature, EVP_sha256); // crash EXC_BAD_ACCESS (code=1, address=0x8)
	if (!rsa.verify("testing", signature, EVP_sha256)) { // crash EXC_BAD_ACCESS (code=1, address=0x60)
		printf("Failed to verify\n");
	}

	return 0;
}

#else

#include <openssl/rsa.h>
#include <openssl/pem.h>

int main(int argc, char* argv[]) {
	const int kBits = 1024;
	const int kExp = 3;

	int keylen;
	char *pem_key;

	RSA *rsa = RSA_generate_key(kBits, kExp, 0, 0);

	/* To get the C-string PEM form: */
	BIO *bio = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);

	keylen = BIO_pending(bio);
	pem_key = reinterpret_cast<char*>(calloc(keylen+1, 1)); /* Null-terminate */
	BIO_read(bio, pem_key, keylen);

	printf("%s", pem_key);

	BIO_free_all(bio);
	RSA_free(rsa);
	free(pem_key);
}

#endif
