#include "os/Statistics.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(int /*argc*/, char * /*argv*/[]) {
	int	iterations= 120000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		try {
			double mean = -1, sum=-1, variance=-1, standardDeviation=-1;

			math::statistics({10, 8, 10, 8, 8, 4}, mean, sum, variance, standardDeviation);
			printf("mean = %0.3f sum = %0.3f variance = %0.3f stddev = %0.3f\n", mean, sum, variance, standardDeviation);
			dotest(abs(mean - 8) < 0.001);
			dotest(abs(sum - 48) < 0.001);
			dotest(abs(variance - 4.8) < 0.001);
			dotest(abs(standardDeviation - 2.191) < 0.001);

			dotest(abs(math::sum({10, 8, 10, 8, 8, 4}) - 48) < 0.001);
			dotest(abs(math::mean({10, 8, 10, 8, 8, 4}) - 8) < 0.001);
			dotest(abs(math::variance({10, 8, 10, 8, 8, 4}) - 4.8) < 0.001);
			dotest(abs(math::stddev({10, 8, 10, 8, 8, 4}) - 2.191) < 0.001);

			try {
				math::statistics({}, mean, sum, variance, standardDeviation);
				dotest(false);
			} catch(const std::out_of_range &exception) {
				printf("We got the exception we expected: %s\n", exception.what());
			}

			try {
				math::statistics({1}, mean, sum, variance, standardDeviation);
				dotest(false);
			} catch(const std::out_of_range &exception) {
				printf("We got the exception we expected: %s\n", exception.what());
			}

			try {
				mean = math::mean({});
				dotest(false);
			} catch(const std::out_of_range &exception) {
				printf("We got the exception we expected: %s\n", exception.what());
			}


			try {
				mean = math::variance({});
				dotest(false);
			} catch(const std::out_of_range &exception) {
				printf("We got the exception we expected: %s\n", exception.what());
			}

			try {
				mean = math::variance({1});
				dotest(false);
			} catch(const std::out_of_range &exception) {
				printf("We got the exception we expected: %s\n", exception.what());
			}

			dotest(abs(math::mean({1}) - 1) < 0.001);

			math::statistics({1, 2}, mean, sum, variance, standardDeviation);
			printf("mean = %0.3f sum = %0.3f variance = %0.3f stddev = %0.3f\n", mean, sum, variance, standardDeviation);
			dotest(abs(mean - 1.5) < 0.001);
			dotest(abs(sum - 3) < 0.001);
			dotest(abs(variance - 0.5) < 0.001);
			dotest(abs(standardDeviation - 0.707) < 0.001);

			dotest(abs(math::sum({1, 2}) - 3) < 0.001);
			dotest(abs(math::mean({1, 2}) - 1.5) < 0.001);
			dotest(abs(math::variance({1, 2}) - 0.5) < 0.001);
			dotest(abs(math::stddev({1, 2}) - 0.707) < 0.001);
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
