#include <stdio.h>
#include "os/Library.h"

int main(const int argc, const char * const argv[]) {
	int	iterations= 500;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		typedef const char *(*zlibVersion)();
		typedef int (*tick_count)();
		for(int arg= 1; arg + 1 < argc; arg+= 2) {
			try	{
				printf("%s:%s->%p\n",argv[arg], argv[arg+1], sys::Library(argv[arg]).function<void*>(argv[arg+1]));
			} catch(const std::exception &exception) {
				fprintf(stderr, "EXCEPTION[arg=%d (\"%s\", \"%s\")]: %s\n", arg, argv[arg], argv[arg+1], exception.what());
			}
		}
		try	{
			printf("z:zlibVersion=%s\n", sys::Library("z").function<zlibVersion>("zlibVersion")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(z:zlibVersion): %s\n", exception.what());
		}
		try	{
			printf("libz:zlibVersion=%s\n", sys::Library("libz").function<zlibVersion>("zlibVersion")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(libz:zlibVersion): %s\n", exception.what());
		}
		try	{
			printf("z.dylib:zlibVersion=%s\n", sys::Library("z.dylib").function<zlibVersion>("zlibVersion")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(z.dylib:zlibVersion): %s\n", exception.what());
		}
		try	{
			printf("libz.dylib:zlibVersion=%s\n", sys::Library("libz.dylib").function<zlibVersion>("zlibVersion")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(libz.dylib:zlibVersion): %s\n", exception.what());
		}
		try	{
			printf("libz.dylib:zlibVersion=%s\n", sys::Library("/usr/lib/libz.dylib").function<zlibVersion>("zlibVersion")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(libz.dylib:zlibVersion): %s\n", exception.what());
		}
		try	{
			printf("Carbon:TickCount=%d\n", sys::Library("Carbon").function<tick_count>("TickCount")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(Carbon:TickCount): %s\n", exception.what());
		}
		try	{
			printf("Carbon.framework:TickCount=%d\n", sys::Library("Carbon.framework").function<tick_count>("TickCount")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(Carbon.framework:TickCount): %s\n", exception.what());
		}
		try	{
			printf("Carbon.framework:TickCount=%d\n", sys::Library("/System/Library/Frameworks/Carbon.framework").function<tick_count>("TickCount")());
		} catch(const std::exception &exception) {
			fprintf(stderr, "EXCEPTION(Carbon.framework:TickCount): %s\n", exception.what());
		}
	}
	return 0;
}
