#include <stdio.h>
#include "os/AllocatableFile.h"

int main(const int /*argc*/, const char * const /*argv*/[]) {
	try	{
		io::AllocatableFile	test("/tmp/test1.bin");
		off_t				locations[]= {
			test.allocate(4),
			test.allocate(3),
			test.allocate(2),
		};
		test.write(std::string("test"), locations[0], io::File::FromStart);
		test.write(std::string("now"), locations[1], io::File::FromStart);
		test.write(std::string("me"), locations[2], io::File::FromStart);
	} catch(const std::exception &exception) {
		fprintf(stderr, "EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
