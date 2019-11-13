#include <stdio.h>
#include "os/MemoryMappedFile.h"
#include "os/FileDescriptor.h"
#include "os/Path.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int argc, const char * const argv[]) {
	int	iterations= 4000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	const io::Path	kTestFilePath(argc < 2 ? "bin/logs/testMemoryMappedFile.txt" : argv[1]);
	const std::string kTestFileContents("Testing memory mapped file.");

	for (int i = 0; i < iterations; ++i) {
		if (kTestFilePath.isFile()) {
			kTestFilePath.remove();
		}

		{
			io::FileDescriptor		file(kTestFilePath);

			file.resize(1024);

			io::MemoryMappedFile	data(file);

			file.close();

			char * const buffer = data.address<char>();

			strcpy(buffer, "Testing memory mapped file.");
		}

		{
			io::MemoryMappedFile	data(kTestFilePath);
			char *buffer = reinterpret_cast<char*>((void*)data);;

			dotest(kTestFileContents == std::string(buffer));
		}
	}
	return 0;
}
