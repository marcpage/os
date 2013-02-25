#include "os/File.h"

int main(int argc,const char * const argv[]) {
	for(int i= 0; i < 10000; ++i) {
		const char * const	kTestFilePath= argc < 2 ? "bin/logs/testFile.txt" : argv[1];
		io::File	test(kTestFilePath, io::File::Binary, io::File::ReadWrite);

		test.moveto(0, io::File::FromEnd);
		test.moveto(1024, io::File::FromHere);
		test.write("Testing");
	}
	return 0;
}
