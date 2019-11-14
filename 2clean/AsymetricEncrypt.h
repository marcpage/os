SecKeyRef publicKey, privateKey;
OSStatus res;
int keySize = 2048;

CFMutableDictionaryRef attrDict = CFDictionaryCreateMutable(NULL, 3, &kCFTypeDictionaryKeyCallBacks, NULL);
CFDictionaryAddValue(attrDict, kSecAttrKeyType, kSecAttrKeyTypeRSA);
CFDictionaryAddValue(attrDict, kSecAttrKeySizeInBits, CFNumberCreate(NULL, kCFNumberIntType, &keySize));

res = SecKeyGeneratePair(attrDict, &publicKey, &privateKey);

OSStatus SecItemExport(CFTypeRef secItemOrArray, SecExternalFormat outputFormat, SecItemImportExportFlags flags, const SecItemImportExportKeyParameters *keyParams, CFDataRef  _Nullable *exportedData);
OSStatus SecItemImport(CFDataRef importedData, CFStringRef fileNameOrExtension, SecExternalFormat *inputFormat, SecExternalItemType *itemType, SecItemImportExportFlags flags, const SecItemImportExportKeyParameters *keyParams, SecKeychainRef importKeychain, CFArrayRef  _Nullable *outItems);
