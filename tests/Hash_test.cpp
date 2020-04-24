#include "os/Hash.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(int /*argc*/, char * /*argv*/[]) {
  int iterations = 50000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    try {
      std::string hash;
      std::string otherHash;

      hash::sha256 test("test", 4);

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
#endif
      dotest(
          hash::sha256("", 0).hex(hash) ==
          "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
      dotest(hash::sha256("test", 4) == hash::sha256("test", 4));
      dotest(
          hash::sha256("test", 4).hex(hash) ==
          "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
      dotest(hash::sha256("test", 4) == hash::sha256::fromHex(hash));
      dotest(hash::sha256::fromHex(hash).valid());
      dotest(hash::sha256("test", 4).valid());
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
