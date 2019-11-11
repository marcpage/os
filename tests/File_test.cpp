#include <stdio.h>
#include "os/File.h"

int main(int argc,const char * const argv[]) {
	int	iterations= 250;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		const char * const	kTestFilePath= argc < 2 ? "bin/logs/testFile.txt" : argv[1];
		io::File	test(kTestFilePath, io::File::Binary, io::File::ReadWrite);

		test.moveto(0, io::File::FromEnd);
		test.moveto(1024, io::File::FromHere);
		test.write("Testing");

		io::File	source("File.h", io::File::Text, io::File::ReadOnly);
		std::string	line;

		do	{
			printf("%s\n", source.readline(line, 0, io::File::FromHere, 16).c_str());
		} while(line.size() > 0);
	}
	return 0;
}
