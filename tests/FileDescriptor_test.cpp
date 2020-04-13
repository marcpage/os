#include "os/FileDescriptor.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int argc, const char *const argv[]) {
  int iterations = 4500;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const char *const kTestFilePath =
      argc < 2 ? "bin/logs/testFileDescriptor.txt" : argv[1];

  for (int i = 0; i < iterations; ++i) {
    io::FileDescriptor file(kTestFilePath);

    file.resize(1024);
    dotest(file.size() == 1024);
    file.resize(2048);
    dotest(file.size() == 2048);
    file.resize(512);
    dotest(file.size() == 512);
    file.close();
    try {
      file.resize(4096);
      dotest(true);
    } catch (const std::exception &exception) {
    }
  }
  return 0;
}

/*
 */
