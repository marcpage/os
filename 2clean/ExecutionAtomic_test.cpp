#include "ExecutionAtomic.h"
#include <stdio.h>

// g++ -o /tmp/test ExecutionAtomic_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

ExecutionAtomicInitialize;

#define ASSERT(test) \
	if(!(test)) { \
		printf("FAILED: %s\n", #test); \
	}

int main(int argc, const char * const argv[]) {
	exec::AtomicNumber	a,b, c(5);
	
	ASSERT(0 == a++);
	ASSERT(0 == b++);
	ASSERT(5 == c);
	return 0;
}
