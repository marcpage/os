#include "../File.h"

// g++ -o /tmp/test File_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

int main(int,const char*[]) {
	io::File	test("/tmp/testFile.txt", io::File::Binary, io::File::ReadWrite);
	
	test.moveto(0, io::File::FromEnd);
	test.moveto(1024, io::File::FromHere);
	test.write("Testing");
	return 0;
}
