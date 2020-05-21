#include "os/ZCompression.h"
#include <stdio.h>
#include <string.h> // strlen

int main(const int, const char *const[]) {
  int iterations = 100;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  const char *testCases[] = {" ",
                             ".",
                             "~",
                             "test",
                             "testing",
                             "t",
                             "tt",
                             "ttt",
                             "tttt",
                             "ttttt",
                             "tttttt",
                             "ttttttt",
                             "tttttttt",
                             "ttttttttt",
                             "The quick brown fox jumped over the lazy dog. "
                             "And then the dog jumped up and bit the fox.",
                             ""};
  std::string source;
  std::string compressed;
  std::string uncompressed;

  for (int i = 0; i < iterations; ++i) {
    for (unsigned int index = 0;
         index < sizeof(testCases) / sizeof(testCases[0]); ++index) {
      source = testCases[index];
      z::compress(source, compressed);
      z::uncompress(compressed, uncompressed);
      if (uncompressed != source) {
        printf("FAILED: iteration %d[%d] source '%s' -> '%s'\n", index, i,
               source.c_str(), uncompressed.c_str());
      }
    }
    while (source.size() < 1024 * 1024) {
      source += "Nobody inspects the spammish repetition. ";
    }
    if (strlen(z::Exception(Z_OK, __FILE__, __LINE__).what()) == 0) {
      printf("FAILED: Z_OK is empty\n");
    }
    if (strlen(z::Exception(-10000, __FILE__, __LINE__).what()) == 0) {
      printf("FAILED: -10000 is empty\n");
    }
    if (strlen(z::Exception(Z_MEM_ERROR, __FILE__, __LINE__).what()) == 0) {
      printf("FAILED: Z_MEM_ERROR is empty\n");
    }
    try {
      z::compress(source, compressed);
      z::uncompress(compressed, uncompressed);
      printf("FAILED: We should have thrown an exception for size being to "
             "small\n");
    } catch (const z::Exception &exception) {
      printf("Correctly caught exception: %s\n", exception.what());
    } catch (const std::exception &exception) {
      printf("FAILED: We threw an unexpected exception (small size): %s\n",
             exception.what());
    }
    z::compress(source, compressed);
    z::uncompress(compressed, uncompressed, source.size());
    if (uncompressed != source) {
      printf("FAILED: \n---- source ----\n%s\n------ destination "
             "-----\n%s\n------------\n",
             source.c_str(), uncompressed.c_str());
    }
    for (int level = 0; level <= 9; ++level) {
      for (unsigned int index = 0;
           index < sizeof(testCases) / sizeof(testCases[0]); ++index) {
        source = testCases[index];
        z::compress(source, compressed, level);
        z::uncompress(compressed, uncompressed);
        if (uncompressed != source) {
          printf("FAILED: iteration %d[%d] source '%s' -> '%s'\n", index, i,
                 source.c_str(), uncompressed.c_str());
        }
      }
    }
    try {
      z::compress(source, compressed, 10);
      printf("FAILED: We should have thrown an exception for bad level (10)\n");
    } catch (const z::Exception &) {
    } catch (const std::exception &exception) {
      printf("FAILED: We threw an unexpected exception (10): %s\n",
             exception.what());
    }
    try {
      z::uncompress(source, compressed);
      printf("FAILED: We should have thrown an exception for corrupted "
             "compressed data\n");
    } catch (const z::Exception &) {
    } catch (const std::exception &exception) {
      printf("FAILED: We threw an unexpected exception (corrupted): %s\n",
             exception.what());
    }
  }
  return 0;
}
