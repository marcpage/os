#include "os/Text.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

/*
d0 ac 20 d1 8c
d0 ad 20 d1 8d
d0 ae 20 d1 8e
d0 af 20 d1 8f
d1 a0 20 d1 a1
d1 a2 20 d1 a3
d1 a4 20 d1 a5
d1 a6 20 d1 a7
d1 a8 20 d1 a9
d1 aa 20 d1 ab
d1 ac 20 d1 ad
d1 ae 20 d1 af
d1 b0 20 d1 b1
d1 b2 20 d1 b3
d1 b4 20 d1 b5
d1 b6 20 d1 b7
d1 b8 20 d1 b9
d1 ba 20 d1 bb
d1 bc 20 d1 bd
d1 be 20 d1 bf
d2 80 20 d2 81
d2 8a 20 d2 8b
d2 8c 20 d2 8d
d2 8e 20 d2 8f
d2 90 20 d2 91
d2 92 20 d2 93
d2 94 20 d2 95
d2 96 20 d2 97
d2 98 20 d2 99
d2 9a 20 d2 9b
d2 9c 20 d2 9d
d2 9e 20 d2 9f
d2 a0 20 d2 a1
d2 a2 20 d2 a3
*/
int main(const int /*argc*/, const char *const /*argv*/[]) {
  int iterations = 2;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const char *strings[] = {
      "test",     "test",     // no change
      "Test",     "test",     // first character
      "TEST",     "test",     // all characters
      //"\xd0\xac", "\xd1\x8c", // utf8
  };
  for (int i = 0; i < iterations; ++i) {
    for (int j = 0; j < int(sizeof(strings) / sizeof(strings[0]) / 2); ++j) {
      std::string mixed = strings[2 * j];
      std::string lower = strings[2 * j + 1];

      if (text::tolower(mixed) != lower) {
        printf("FAIL: '%s' -> '%s' but got '%s'\n", mixed.c_str(),
               lower.c_str(), text::tolower(mixed).c_str());
      }
    }
  }
  return 0;
}
