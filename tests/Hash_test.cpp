#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

template <class T>
void testConstness(const T &hash, const std::string &valueToHash) {
  std::string rawHash(reinterpret_cast<const char *>(hash.buffer()),
                      hash.size());
  T valueHash(valueToHash);
  std::string valueHashRaw(reinterpret_cast<const char *>(valueHash.buffer()),
                           valueHash.size());
  dotest(rawHash == valueHashRaw);
}

int main(int /*argc*/, char * /*argv*/[]) {
  int iterations = 13000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    try {
      std::string hashValue;
      std::string otherHash;

      hash::sha256 test("test", 4), testing("testing");

      testConstness(test, "test");
      testConstness(testing, "testing");
#if __APPLE_CC__ || __APPLE__
      printf("Testing Mac specific code\n");
      dotest(std::string("sha256") == hash::CommonCryptoSHA256Hasher().name());
#endif
#if OpenSSLAvailable
      printf("Testing openssl specific code\n");
      dotest(std::string("sha256") == hash::OpenSSLSHA256Hasher().name());
      dotest(std::string("md5") == hash::OpenSLLMD5Hasher().name());
#endif
#if (__APPLE_CC__ || __APPLE__) && OpenSSLAvailable
      printf("Testing openssl Mac specific code\n");
      dotest(hash::sha256("test", 4).hex() ==
             hash::openssl_sha256("test", 4).hex(otherHash));
      dotest(hash::sha256("", 0).hex() ==
             hash::openssl_sha256("", 0).hex(otherHash));
      dotest(hash::openssl_sha256("", 0).hex() ==
             std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca4959"
                         "91b7852b855"));
      dotest(hash::openssl_sha256("", 0).base64() ==
             std::string("47DEQpj8HBSa-_TImW-5JCeuQeRkm5NMpJWZG3hSuFU"));

#endif
      dotest(
          hash::sha256("", 0).hex(hashValue) ==
          "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
      dotest(hash::sha256("test", 4) == hash::sha256("test", 4));
      dotest(
          hash::sha256("test", 4).hex(hashValue) ==
          "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
      dotest(hash::sha256("test", 4) == hash::sha256::fromHex(hashValue));
      dotest(hash::sha256::fromHex(hashValue).valid());
      dotest(hash::sha256("test", 4).base64(hashValue) ==
             "n4bQgYhMfWWaL-qgxVrQFaO_TxsrC4Is0V1sFbDwCgg");
      dotest(hash::sha256("test", 4) == hash::sha256::fromBase64(hashValue));
      dotest(hash::sha256::fromBase64(hashValue).valid());
      dotest(hash::sha256("test", 4).valid());
      dotest(test == hash::sha256("test"));
      dotest(bool(test));
      dotest(!bool(hash::sha256()));
      dotest(test.size() == testing.size());
      hashValue.assign(reinterpret_cast<const char *>(test.buffer()),
                       test.size());
      otherHash.assign(
          reinterpret_cast<const char *>(hash::sha256("what").buffer()),
          hash::sha256("what").size());
      dotest(test != testing);
      dotest((hash::sha256(test) = hash::sha256("other")) ==
             hash::sha256("other"));
      dotest(test == hash::sha256("test"));
      dotest(test != hash::sha256("other"));
      test = hash::sha256("hello", 5);
      dotest(hash::sha256("hello", 5) == test);
      test.reset("test", 4);
      dotest(hash::sha256("test", 4) == test);
      dotest(!hash::sha256().valid());
      dotest(hash::sha256("test", 4).size() == 32);
      dotest(std::string(hash::sha256().name()) == "sha256");
      test.reset(std::string("working"));
      dotest(hash::sha256("working", 7) == test);
      dotest(hash::sha256::fromData(test.buffer(), test.size()) == test);
      try {
        hash::sha256::fromData(test.buffer(), 3);
        dotest(false);
      } catch (const msg::Exception &) {
      }
    } catch (const std::exception &exception) {
      printf("FAILED: Exception: %s\n", exception.what());
    }
  }
  return 0;
}
