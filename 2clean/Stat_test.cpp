#include "Stat.h"
#include <stdio.h>

// g++ -o /tmp/test Stat_test.cpp -I.. -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test /* /tmp/*

static void print(const io::Stat &info) {
	std::string	buffer, another;
	
	printf("Path: %s\n", info.path().c_str());
	printf(" Owner: %s (%d)\n", info.user(buffer).c_str(), info.uid());
	printf(" Group: %s (%d)\n", info.group(buffer).c_str(), info.gid());
	printf(" Size: %d\n", static_cast<int>(info.size()));
	printf(" Allocated: %d\n", static_cast<int>(info.allocated()));
	printf(" Block Size: %d\n", static_cast<int>(info.blocksize()));
	printf(" Blocks: %d\n", static_cast<int>(info.blocks()));
	printf(" Modification Time: %s (%s GMT) (%d)\n", info.modified(buffer, io::Stat::Local).c_str(), info.modified(another, io::Stat::GMT).c_str(), static_cast<int>(info.modified()));
	printf(" Access Time: %s (%s GMT) (%d)\n", info.access(buffer, io::Stat::Local).c_str(), info.access(another, io::Stat::GMT).c_str(), static_cast<int>(info.access()));
	printf(" Status Time: %s (%s GMT) (%d)\n", info.status(buffer, io::Stat::Local).c_str(), info.status(another, io::Stat::GMT).c_str(), static_cast<int>(info.status()));
	printf(" Mode: %08x\n", static_cast<int>(info.mode()));
	printf(" Read Only: %s\n", info.readonly() ? "true" : "false");
	printf(" Executable: %s\n", info.executable() ? "true" : "false");
	printf(" Writable: %s\n", info.writable() ? "true" : "false");
	printf(" Readable: %s\n", info.readable() ? "true" : "false");
	printf(" File: %s\n", info.file() ? "true" : "false");
	printf(" Directory: %s\n", info.directory() ? "true" : "false");
	printf(" Link: %s\n", info.link() ? "true" : "false");
	if(info.link()) {
		printf(" Target: %s\n", info.readlink(buffer).c_str());
		print(io::Stat(info.path().c_str()));
	}
	if(info.directory()) {
		std::vector<std::string>	files, directories;
		
		info.list(&files, &directories, io::Stat::AllFiles);
		for(std::vector<std::string>::iterator iterator= files.begin(); iterator != files.end(); ++iterator) {
			printf(" * %s\n", iterator->c_str());
		}
		for(std::vector<std::string>::iterator iterator= directories.begin(); iterator != directories.end(); ++iterator) {
			printf(" > %s\n", iterator->c_str());
		}
	}
}

int main(int argc, char *argv[]) {
	try {
		for(int arg= 1; arg < argc; ++arg) {
			print(io::Stat(argv[arg], io::Stat::LookAtSymlink));
		}
	} catch(const std::exception &exception) {
		printf("FAILED: Exception: %s\n", exception.what());
	}
	return 0;
}
