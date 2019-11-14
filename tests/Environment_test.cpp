#include <stdio.h>
#include "os/Environment.h"

#define dotest(condition) \
	if(!(condition)) { \
		fprintf(stderr, "FAIL(%s:%d): %s\n",__FILE__, __LINE__, #condition); \
	}

int main(const int , const char * const []) {
	int	iterations= 7500;
#ifdef __Tracer_h__
	iterations= 1;
#endif
  for (const char **env = environ; *env != nullptr; env++)
  {
    const char *thisEnv = *env;
    printf("%s\n", thisEnv);
  }
	for (int i = 0; i < iterations; ++i) {
		env::Dictionary initialList, laterList;
		env::list(initialList);
		printf("initialList\n");
		for (const auto& kv : initialList) {
			printf("%s = '%s'\n", kv.first.c_str(), kv.second.c_str());
		}
		env::set("Testing_Env_Stuff", "1");
		env::list(laterList);
		printf("laterList\n");
		for (const auto& kv : laterList) {
			printf("%s = '%s'\n", kv.first.c_str(), kv.second.c_str());
		}
		for (const auto& kv : laterList) {
			printf("%s = '%s'\n", kv.first.c_str(), kv.second.c_str());
		}
		dotest(initialList.size() + 1 == laterList.size());
		printf("initialList = %lu laterList = %lu\n", initialList.size(), laterList.size());
		dotest(env::get("Testing_Env_Stuff") == "1");
		dotest(env::has("Testing_Env_Stuff"));
		env::clear("Testing_Env_Stuff");
		dotest(!env::has("Testing_Env_Stuff"));
		dotest(env::get("Testing_Env_Stuff") == "");
		env::list(laterList);
		printf("laterList\n");
		printf("laterList\n");
		for (const auto& kv : laterList) {
			printf("%s = '%s'\n", kv.first.c_str(), kv.second.c_str());
		}
		dotest(initialList.size() == laterList.size());
	}
	return 0;
}
