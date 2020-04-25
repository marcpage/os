#include "os/File.h"
#include "os/Path.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(int argc, const char *const argv[]) {
  int iterations = 200;
  const char *const lines[] = {
      "a\r\n", "b\r", "c\n", "d\r\n", "e\r", "f\n", "g\r\n", "h\r", "i\n",
      "j\r\n", "k\r", "l\n", "m\r\n", "n\r", "o\n", "p\r\n", "q\r", "r\n",
      "s\r\n", "t\r", "u\n", "v\r\n", "w\r", "x\n", "y\r\n", "z\r",
  };
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    const char *const kTestFilePath =
        argc < 2 ? "bin/logs/testFile.txt" : argv[1];
    io::File test(kTestFilePath, io::File::Binary, io::File::ReadWrite);
    std::string buffer;
    std::string line;
    std::string contents;

    dotest(test.writable());

    test.moveto(0, io::File::FromEnd);
    test.moveto(1024, io::File::FromHere);
    test.write("Testing");
    test.flush();

    io::Path binaryFilePath =
        (io::Path::working() + "bin" + "logs").uniqueName("numbers_", ".bin");
    auto *binary = new io::File(binaryFilePath, io::File::Binary,
                                io::File::WriteIfPossible);

    dotest(binary->writable());
    binary->write<uint8_t>(0, io::File::BigEndian);
    binary->write<uint8_t>(1, io::File::BigEndian);
    binary->write<uint8_t>(2, io::File::LittleEndian);
    binary->write<uint8_t>(3, io::File::NativeEndian);
    binary->write<uint16_t>(4, io::File::BigEndian);
    binary->write<uint16_t>(5, io::File::LittleEndian);
    binary->write<uint16_t>(6, io::File::NativeEndian);
    binary->write<uint32_t>(7, io::File::BigEndian);
    binary->write<uint32_t>(8, io::File::LittleEndian);
    binary->write<uint32_t>(9, io::File::NativeEndian);
    binary->write<uint64_t>(10, io::File::BigEndian);
    binary->write<uint64_t>(11, io::File::LittleEndian);
    binary->write<uint64_t>(12, io::File::NativeEndian);
    binary->move(0, io::File::FromStart);
    binary->write<uint8_t>(13, io::File::NativeEndian);
    binary->flush();
    delete binary;
    binary = nullptr;

    binary = new io::File(binaryFilePath, io::File::Binary, io::File::ReadOnly);
    dotest(!binary->writable());
    dotest(binary->read<uint8_t>(io::File::NativeEndian) == 13);
    dotest(binary->read<uint8_t>(io::File::BigEndian) == 1);
    dotest(binary->read<uint8_t>(io::File::LittleEndian) == 2);
    dotest(binary->read<uint8_t>(io::File::NativeEndian) == 3);
    dotest(binary->read<uint16_t>(io::File::BigEndian) == 4);
    dotest(binary->read<uint16_t>(io::File::LittleEndian) == 5);
    dotest(binary->read<uint16_t>(io::File::NativeEndian) == 6);
    dotest(binary->read<uint32_t>(io::File::BigEndian) == 7);
    dotest(binary->read<uint32_t>(io::File::LittleEndian) == 8);
    dotest(binary->read<uint32_t>(io::File::NativeEndian) == 9);
    dotest(binary->read<uint64_t>(io::File::BigEndian) == 10);
    dotest(binary->read<uint64_t>(io::File::LittleEndian) == 11);
    dotest(binary->read<uint64_t>(io::File::NativeEndian) == 12);
    delete binary;
    binary = nullptr;

    binaryFilePath =
        (io::Path::working() + "bin" + "logs").uniqueName("eols_", ".txt");
    binary =
        new io::File(binaryFilePath, io::File::Text, io::File::WriteIfPossible);
    for (auto index = 0;
         index < static_cast<decltype(index)>(sizeof(lines) / sizeof(lines[0]));
         ++index) {
      binary->write(lines[index]);
    }
    delete binary;
    binary = nullptr;

    binary = new io::File(binaryFilePath, io::File::Text, io::File::ReadOnly);
    for (auto index = 0;
         index < static_cast<decltype(index)>(sizeof(lines) / sizeof(lines[0]));
         ++index) {
      dotest(binary->readline(buffer, 0, io::File::FromHere, 2) ==
             lines[index]);
    }
    delete binary;
    binary = nullptr;

    io::File source("File.h", io::File::Text, io::File::ReadOnly);

    do {
      contents += source.readline(line, 0, io::File::FromHere, 16);
    } while (line.size() > 0);

    dotest(contents ==
           io::File("File.h", io::File::Text, io::File::ReadOnly).read(buffer));
  }
  return 0;
}
