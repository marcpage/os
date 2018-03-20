#include <stdio.h>
#include <string>
#include <os/File.h>
#include <os/Path.h>

int main(int ,const char * const []) {
	int	iterations= 3000;
#ifdef __Tracer_h__
	iterations= 1;
#endif
	for(int i= 0; i < iterations; ++i) {
		try {
			io::Path working("bin/logs//");
			io::Path test(working + "Path_test_file.txt");

			if (!test.parent().isDirectory()) {
				printf("FAILED: Parent directory does not exist: %s\n", std::string(test).c_str());
			}
			if (test.isAbsolute()) {
				printf("FAILED: Should not be absolute: %s\n", std::string(test).c_str());
			}
			if (test.isEmpty()) {
				printf("FAILED: Should not be empty: %s\n", std::string(test).c_str());
			}
			if (!test.isRelative()) {
				printf("FAILED: Should be relative: %s\n", std::string(test).c_str());
			}

			if (test.isFile()) {
				test.remove();
			}
			{io::File(test, io::File::Text, io::File::ReadWrite).write("test");}
			if (!test.isFile()) {
				printf("FAILED: Created %s but it isn't a file\n", std::string(test).c_str());
			}
			if (test.isDirectory()) {
				printf("FAILED: Created %s as a file but it is a directory\n", std::string(test).c_str());
			}
			if (test.isLink()) {
				printf("FAILED: Created %s as a file but it is a link\n", std::string(test).c_str());
			}
			test.remove();
			if (test.isFile()) {
				printf("FAILED: Deleted %s but the file is still there\n", std::string(test).c_str());
			}
			if (test.exists()) {
				printf("FAILED: Deleted %s but it is still there\n", std::string(test).c_str());
			}
			if (test.isLink()) {
				printf("FAILED: Deleted %s but a link is there\n", std::string(test).c_str());
			}

			test = working + "Path_test_directory";
			if (test.isDirectory()) {
				test.remove();
			}
			test.mkdir();
			if (test.isFile()) {
				printf("FAILED: Created %s as a directory but it is a file\n", std::string(test).c_str());
			}
			if (!test.isDirectory()) {
				printf("FAILED: Created %s but it is not a directory\n", std::string(test).c_str());
			}
			if (test.isLink()) {
				printf("FAILED: Created %s as a directory but it is a link\n", std::string(test).c_str());
			}
			test.remove();
			if (test.isDirectory()) {
				printf("FAILED: Deleted %s but the directory is still there\n", std::string(test).c_str());
			}
			if (test.exists()) {
				printf("FAILED: Deleted %s but it is still there\n", std::string(test).c_str());
			}
			if (test.isLink()) {
				printf("FAILED: Deleted %s but a link is there\n", std::string(test).c_str());
			}
			if (!io::Path("test").parent().isEmpty()) {
				printf("FAILED: 'test' parent is not empty\n");
			}
			if (std::string(io::Path("/test").parent()) != "/") {
				printf("FAILED: '/test' parent is not '/'\n");
			}
		} catch(const std::exception &exception) {
			printf("FAILED: Exception: %s\n", exception.what());
		}
	}
	return 0;
}
