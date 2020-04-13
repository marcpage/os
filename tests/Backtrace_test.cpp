#include "os/Backtrace.h"
#include "os/Exception.h"
#include <stdio.h>

int main(const int, const char *const[]) {
  int iterations = 100000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    try {
      ThrowMessageException("Testing");
    } catch (const std::exception &e) {
      printf("%s\n", e.what());
    }
  }
  return 0;
}
