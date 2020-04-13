#include "os/FileDescriptor.h"
#include "os/MemoryMappedFile.h"
#include "os/Path.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int argc, const char *const argv[]) {
  int iterations = 4000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const io::Path kTestFilePath(argc < 2 ? "bin/logs/testMemoryMappedFile.txt"
                                        : argv[1]);
  const char *const kTestFileContents = "Testing memory mapped file.";

  for (int i = 0; i < iterations; ++i) {
    if (kTestFilePath.isFile()) {
      kTestFilePath.remove();
    }

    {
      io::FileDescriptor file(kTestFilePath);

      file.resize(1024);

      io::MemoryMappedFile data(file);

      dotest(data.size() == 1024);
      file.close();

      char *const buffer = data.address<char>();

      strcpy(buffer, kTestFileContents);
    }

    {
      io::MemoryMappedFile data(kTestFilePath);
      char *buffer = reinterpret_cast<char *>((void *)data);

      dotest(data.size() == 1024);
      printf("data size = %lu\n", data.size());
      dotest(std::string(kTestFileContents) == std::string(buffer));
    }
  }
  return 0;
}
