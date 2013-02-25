#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "os/Execute.h"
#include "os/DateTime.h"

typedef std::string	String;
typedef std::vector<String>	StringList;

std::map<std::string,std::string>	gCompilerLocations;
const char * const gCompilerFlags= "-I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -lsqlite3 -framework Carbon";

String &stripEOL(String &s) {
	while(s[s.size()-1] == '\n') {
		s.erase(s.size()-1);
	}
}

StringList &split(const String &string, const char character, StringList &parts) {
	String::size_type	characterPos, start= 0;

	parts.clear();
	do	{
		characterPos= string.find(character, start);
		if(String::npos == characterPos) {
			characterPos= string.size();
		}
		parts.push_back(string.substr(start, characterPos - start));
		start= characterPos+1;
	} while(start < string.size());
	return parts;
}

double runNoResultsExpected(const String &command, const char * const action) {
	std::string		results;
	double			duration;
	dt::DateTime	start;

	exec::execute(command, results);
	duration= dt::DateTime() - start;
	if(stripEOL(results).size() != 0) {
		printf("unexpected %s result '%s'\n", action, results.c_str());
	}

	return duration;
}

void runTest(const String &name, const String &compiler, uint32_t testedLines, double durationInSeconds) {
	std::string		results;
	std::string		command;
	std::string		executableName;
	std::string		logName;
	std::string		runLogName;
	dt::DateTime	start, end;
	double			compilePerfTime, compileCoverageTime, runPerfTime, runCoverageTime;
	uint32_t		coverage;

	if(gCompilerLocations[compiler].size() == 0) {
		exec::execute("which "+compiler, results);
		if(stripEOL(results).size() == 0) {
			results= "-";
			printf("Unable to find compiler %s\n", compiler.c_str());
		}
		gCompilerLocations[compiler]= results;
		printf("COMPILER='%s'\n", results.c_str());
	}
	command= gCompilerLocations[compiler];
	if(command != "-") {
		printf("%s - %s\n", name.c_str(), compiler.c_str());
		executableName= name + '_' + compiler + "_performance";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		command+= " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+gCompilerFlags+" &> bin/logs/"+logName;
		compilePerfTime= runNoResultsExpected(command, "compile performance");
		command= executableName+" &> bin/logs/"+runLogName;
		runPerfTime= runNoResultsExpected(command, "run performance");

		executableName= name + '_' + compiler + "_trace";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		command= gCompilerLocations[compiler]
					+ " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+gCompilerFlags+" -include Tracer.h &> bin/logs/"+logName;
		compileCoverageTime= runNoResultsExpected(command, "compile trace");
		command= executableName+" &> bin/logs/"+runLogName;
		runCoverageTime= runNoResultsExpected(command, "run trace");
		exec::execute("cat "+runLogName+" | grep "+executableName+"\\\\.h | sort | uniq | wc -l", results);
		coverage= atoi(results.c_str());
		printf("Coverage %d (expected %d)\n", coverage, testedLines);
	}
}

void runTest(const String &name, const String &compilerResults) {
	StringList	compilers;
	StringList	values;

	split(compilerResults, ';', compilers);
	for(StringList::iterator compilerInfo= compilers.begin(); compilerInfo != compilers.end(); ++compilerInfo) {
		std::string	compiler;
		uint32_t	testedLines;
		double		durationInSeconds;

		split(*compilerInfo, ':', values);
		compiler= values[0];
		testedLines= atoi(values[1].c_str());
		durationInSeconds= atof(values[2].c_str());
		runTest(name, compiler, testedLines, durationInSeconds);
	}
}

int main(int /*argc*/, const char * const /*argv*/[]) {
	struct {
		const char * const	name;
		const char * const	compilerresults;
	} tests[]= {
		{"ArchiveFile",			"clang++:135:0.002;"	"g++:135:0.002;"	"llvm-g++:135:0.002"},
		{"AtomicInteger",		"clang++:12:0.421;"		"g++:12:0.421;"		"llvm-g++:12:0.421"},
		{"Bencode",				"clang++:244:0.002;"	"g++:244:0.002;"	"llvm-g++:244:0.002"},
		{"CompactNumber",		"clang++:15:0.005;"		"g++:15:0.005;"		"llvm-g++:15:0.005"},
		{"DateTime",			"clang++:12:0.007;"		"g++:12:0.007;"		"llvm-g++:12:0.007"},
		{"EnumSet",				"clang++:191:0.007;"	"g++:191:0.007;"	"llvm-g++:191:0.007"},
		{"Exception",			"clang++:19:0.013;"		"g++:19:0.013;"		"llvm-g++:19:0.013"},
		{"Execute",				"clang++:8:10.949;"		"g++:8:10.949;"		"llvm-g++:8:10.949"},
		{"File",				"clang++:35:0.005;"		"g++:35:0.005;"		"llvm-g++:35:0.005"},
		{"Hash",				"clang++:52:0.004;"		"g++:52:0.004;"		"llvm-g++:52:0.004"},
		{"Library",				"clang++:113:0.248;"	"g++:113:0.248;"	"llvm-g++:113:0.248"},
		{"Mutex",				"clang++:15:0.033;"		"g++:15:0.033;"		"llvm-g++:15:0.033"},
		{"RWLock",				"clang++:18:7.427;"		"g++:18:7.427;"		"llvm-g++:18:7.427"},
		{"ReferenceCounted",	"clang++:200:3.061;"	"g++:200:3.061;"	"llvm-g++:200:3.061"},
		{"ReferencedString",	"clang++:251:0.028;"	"g++:251:0.028;"	"llvm-g++:251:0.028"},
		{"SocketServer",		"clang++:15:7.427;"		"g++:15:7.427;"		"llvm-g++:15:7.427"},
		{"Sqlite3Plus",			"clang++:31:0.324;"		"g++:31:0.324;"		"llvm-g++:31:0.324"},
		{"Thread",				"clang++:27:0.058;"		"g++:27:0.058;"		"llvm-g++:27:0.058"},
	};
	try {
		std::string	results;

		exec::execute("mkdir -p bin/tests bin/logs", results);
		if(results != "") {
			printf("WARNING: mkdir '%s'\n", results.c_str());
		}
		exec::execute("rm -Rf bin/logs/* bin/tests/*", results);
		if(results != "") {
			printf("WARNING: rm '%s'\n", results.c_str());
		}
		for(int test= 0; test < sizeof(tests)/sizeof(tests[0]); ++test) {
			runTest(tests[test].name, tests[test].compilerresults);
		}
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
