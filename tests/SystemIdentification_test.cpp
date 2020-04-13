#include "os/SystemIdentification.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int, const char *const[]) {
  int iterations = 40000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    printf(
        "System Name: %s Node name: %s Release: %s Version: %s Machine: %s\n",
        sys::osName().c_str(), sys::nodeName().c_str(),
        sys::osRelease().c_str(), sys::systemVersion().c_str(),
        sys::architecture().c_str());
  }
  return 0;
}
