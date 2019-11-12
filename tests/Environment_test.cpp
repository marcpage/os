#include <stdio.h>
#include "os/Environment.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int , const char * const []) {
	int	iterations= 100;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for (int i = 0; i < iterations; ++i) {
		env::Dictionary initialList, laterList;
		env::list(initialList);
		env::set("Testing_Env_Stuff", "1");
		env::list(laterList);
		dotest(initialList.size() + 1 == laterList.size());
		printf("initialList = %d laterList = %d\n", initialList.size(), laterList.size());
		dotest(env::get("Testing_Env_Stuff") == "1");
		dotest(env::has("Testing_Env_Stuff"));
		env::clear("Testing_Env_Stuff");
		dotest(!env::has("Testing_Env_Stuff"));
		dotest(env::get("Testing_Env_Stuff") == "");
		env::list(laterList);
		dotest(initialList.size() == laterList.size());
	}
	return 0;
}

/*
		 */
