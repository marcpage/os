#include <stdio.h>
#include "os/DateTime.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	dt::DateTime	t1,t2;

	fprintf(stdout, "construction time: %0.9f\n",t2 - t1);
	return 0;
}
