#if __APPLE_CC__ || __APPLE__
	#include <CommonCrypto/CommonCryptor.h>
	#include <Security/Security.h>
#endif

// clang++ 2clean/AsymetricEncrypt.cpp -framework Security -framework CoreFoundation -o /tmp/test

#include <stdio.h>
#include <string>

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

