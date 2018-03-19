#include <stdio.h>
#include "os/DateTime.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	int	iterations= 60000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		dt::DateTime	t1,t2;

		fprintf(stdout, "construction time: %0.9f\n",t2 - t1);
		fprintf(stdout, "Years to Cocoa Epoch %0.1f\n", AddToConvertToAppleCocoaEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stdout, "Years to DOS Epoch %0.1f\n", AddToConvertToDOSEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stdout, "Years to UNIX Epoch %0.1f\n", AddToConvertToUNIXEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stdout, "Years to Apple Epoch %0.1f\n", AddToConvertToAppleMacEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
	}
	return 0;
}
