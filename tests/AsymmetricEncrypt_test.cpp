#include "os/AsymmetricEncrypt.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d#%d-%d): %s\n", __FILE__, __LINE__,             \
            keySizeIndex, dataIndex, #condition);                              \
  }

int main(int /*argc*/, char * /*argv*/[]) {
  int iterations = 1;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  int keySizes[] = {
      1024,
#ifndef __Tracer_h__
      1032,
      2048,
      4096,
#endif
  };
  unsigned long exponents[] = {
      3,
#ifndef __Tracer_h__
      5, 17, 257, 65537,
#endif
  };
  const char *testSets[] = {
      "", "Testing", "\x01Testing\x01",
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789012345678901"
      "2345678901234567890123456789"};
  const char *dummyHash =
      "0123456789ABCDEDF0123456789abcdef0123456789ABCDEDF0123456789abcdef";
  std::string publicKey, privateKey, signature, buffer;

  for (int i = 0; i < iterations; ++i) {
    for (unsigned int keySizeIndex = 0;
         keySizeIndex < sizeof(keySizes) / sizeof(keySizes[0]);
         ++keySizeIndex) {
      for (unsigned int dataIndex = 0;
           dataIndex < sizeof(testSets) / sizeof(testSets[0]); ++dataIndex) {
        for (unsigned int exponentIndex = 0;
             exponentIndex < sizeof(exponents) / sizeof(exponents[0]);
             ++exponentIndex) {
          try {
            crypto::OpenSSLRSAAES256PrivateKey rsa(keySizes[keySizeIndex],
                                                   exponents[exponentIndex]);
            crypto::AutoClean<crypto::AsymmetricPublicKey> publicRsa(
                rsa.publicKey());

            dotest(publicRsa->verify(testSets[dataIndex],
                                     rsa.sign(testSets[dataIndex], signature)));

            dotest(rsa.decrypt(publicRsa->encrypt(dummyHash, buffer),
                               signature) == dummyHash);

            crypto::OpenSSLRSAAES256PrivateKey rsa2(rsa.serialize(buffer));
            crypto::AutoClean<crypto::AsymmetricPublicKey> publicRsa2(
                rsa.publicKey());

            dotest(rsa.serialize(privateKey) == rsa2.serialize(buffer));
            dotest(publicRsa->serialize(privateKey) ==
                   publicRsa2->serialize(buffer));
            dotest(rsa.serialize(privateKey) != publicRsa->serialize(buffer));

            dotest(
                publicRsa2->verify(testSets[dataIndex],
                                   rsa2.sign(testSets[dataIndex], signature)));
            dotest(publicRsa2->verify(
                testSets[dataIndex], rsa.sign(testSets[dataIndex], signature)));
            dotest(
                publicRsa->verify(testSets[dataIndex],
                                  rsa2.sign(testSets[dataIndex], signature)));

            dotest(!publicRsa->verify(
                testSets[dataIndex],
                rsa2.sign(testSets[dataIndex], signature).substr(1)));

            dotest(rsa2.decrypt(publicRsa2->encrypt(dummyHash, buffer),
                                signature) == dummyHash);
            dotest(rsa.decrypt(publicRsa2->encrypt(dummyHash, buffer),
                               signature) == dummyHash);
            dotest(rsa2.decrypt(publicRsa->encrypt(dummyHash, buffer),
                                signature) == dummyHash);

          } catch (const std::exception &exception) {
            fprintf(stderr, "FAILED(%d:%d): Exception: %s\n", keySizeIndex,
                    dataIndex, exception.what());
          }
        }
      }
    }
  }
  return 0;
}
