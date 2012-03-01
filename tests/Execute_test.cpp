#include "os/Execute.h"
#include <stdio.h>

int main(const int argc,const char*const argv[]) {
	std::string	result;

	for(int arg= 1; arg < argc; ++arg) {
		exec::execute(argv[arg], result, 6);
		printf("[[%s]]\n", result.c_str());
	}
	return 0;
}
