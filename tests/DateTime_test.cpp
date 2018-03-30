#include <stdio.h>
#include "os/DateTime.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int /*argc*/, const char * const /*argv*/[]) {
	int	iterations= 25000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		dt::DateTime	t1,t2;
		std::string		buffer;

		fprintf(stderr, "construction time: %0.9f\n",t2 - t1);
		fprintf(stderr, "Years to Cocoa Epoch %0.1f\n", AddToConvertToAppleCocoaEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stderr, "Years to DOS Epoch %0.1f\n", AddToConvertToDOSEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stderr, "Years to UNIX Epoch %0.1f\n", AddToConvertToUNIXEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
		fprintf(stderr, "Years to Apple Epoch %0.1f\n", AddToConvertToAppleMacEpoch / 365.2525 / 24.0 / 60.0 / 60.0);

		t1= dt::DateTime(2018, dt::DateTime::Feb, 5, 14, 30, 15);
		dotest(t1.format("%A", buffer) == "Monday");
		dotest(t1.format("%a", buffer) == "Mon");
		dotest(t1.format("%B", buffer) == "February");
		dotest(t1.format("%b", buffer) == "Feb");
		dotest(t1.format("%C", buffer) == "20");
		dotest(t1.format("%D", buffer) == "02/05/18");
		dotest(t1.format("%d", buffer) == "05");
		dotest(t1.format("%F", buffer) == "2018-02-05");
		dotest(t1.format("%M", buffer) == "30");
		dotest(t1.format("%m", buffer) == "02");
		dotest(t1.format("%p", buffer) == "PM");
		dotest(t1.format("%S", buffer) == "15");
	}
	return 0;
}
