#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "os/Execute.h"
#include "os/DateTime.h"

struct Times {
	double		perfCompile, perfRun, traceCompile, traceRun;
	uint32_t	testedLines, warnings;
};
typedef std::string							String;
typedef std::vector<String>					StringList;
typedef std::map<std::string,std::string>	Dictionary;
typedef std::map<std::string,Times>			CompilerTimes;
typedef std::map<std::string,CompilerTimes>	TestCompilerTimes;

TestCompilerTimes	compilerTimes;
Dictionary			gCompilerLocations;
const char * const	gCompilerFlags= "-I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -lsqlite3 -framework Carbon";

String &stripEOL(String &s) {
	while(s[s.size()-1] == '\n') {
		s.erase(s.size()-1);
	}
	return s;
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

int runIntegerExpected(const String &command) {
	String	results;

	exec::execute(command, results);
	return atoi(results.c_str());
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

void runTest(const String &name, const String &compiler, uint32_t testedLines, double durationInSeconds, double totalTimeInSeconds) {
	std::string		results;
	std::string		command;
	std::string		executableName;
	std::string		logName;
	std::string		runLogName;
	dt::DateTime	start, end;
	double			compilePerfTime, compileCoverageTime, runPerfTime, runCoverageTime, totalTime;
	uint32_t		coverage;
	uint32_t		warnings;

	if(gCompilerLocations[compiler].size() == 0) {
		exec::execute("which "+compiler, results);
		if(stripEOL(results).size() == 0) {
			results= "-";
			printf("WARNING: Unable to find compiler %s\n", compiler.c_str());
		}
		gCompilerLocations[compiler]= results;
		//printf("COMPILER='%s'\n", results.c_str());
	}
	command= gCompilerLocations[compiler];
	if(command != "-") {
		printf("%-18s %9s about %0.3fs\n", name.c_str(), compiler.c_str(), totalTimeInSeconds);
		executableName= name + '_' + compiler + "_performance";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		command+= " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+gCompilerFlags+" &> bin/logs/"+logName;
		compilePerfTime= runNoResultsExpected(command, "compile performance");
		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runPerfTime= runNoResultsExpected(command, "run performance");

		warnings= runIntegerExpected("cat bin/logs/"+logName+" | grep warning: | sort | uniq | wc -l");
		if(warnings > 0) {
			printf("\t%d Compile Warnings\n", warnings);
		}

		executableName= name + '_' + compiler + "_trace";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		command= gCompilerLocations[compiler]
					+ " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+gCompilerFlags+" -include Tracer.h &> bin/logs/"+logName;
		compileCoverageTime= runNoResultsExpected(command, "compile trace");
		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runCoverageTime= runNoResultsExpected(command, "run trace");

		coverage= runIntegerExpected("cat bin/logs/"+runLogName+" | grep "+name+"\\\\.h | sort | uniq | wc -l");
		if(coverage != testedLines) {
			printf("\tTested Lines: %d Expected %d\n", coverage, testedLines);
		}
		if(runPerfTime > durationInSeconds) {
			printf("\tTest took %0.3fs seconds, expected %0.3fs seconds\n", runPerfTime, durationInSeconds);
		}
		totalTime= compilePerfTime + compileCoverageTime + runPerfTime + runCoverageTime;
		if(totalTime > totalTimeInSeconds) {
			printf("\tBuild/Test took %0.3fs seconds, expected %0.3fs seconds\n", totalTime, totalTimeInSeconds);
		}
#if 0
		printf("Coverage %d (expected %d) Compile: %0.3fs Run: %0.3fs (expected %0.3fs) Trace Compile: %0.3fs Trace Run: %0.3fs\n",
			coverage, testedLines,
			compilePerfTime, runPerfTime, durationInSeconds, compileCoverageTime, runCoverageTime
		);
#endif
		compilerTimes[name][compiler].perfCompile= compilePerfTime;
		compilerTimes[name][compiler].perfRun= runPerfTime;
		compilerTimes[name][compiler].traceCompile= compileCoverageTime;
		compilerTimes[name][compiler].traceRun= runCoverageTime;
		compilerTimes[name][compiler].testedLines= coverage;
		compilerTimes[name][compiler].warnings= warnings;
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
		double		totalTimeInSeconds;

		split(*compilerInfo, ':', values);
		compiler= values[0];
		testedLines= atoi(values[1].c_str());
		durationInSeconds= atof(values[2].c_str());
		totalTimeInSeconds= atof(values[3].c_str());
		runTest(name, compiler, testedLines, durationInSeconds, totalTimeInSeconds);
	}
}

bool isIntel() {
	std::string	results;

	exec::execute("uname -p", results);
	if(stripEOL(results) == "i386") {
		return true;
	}
	return false;
}
/**
	@todo Add debug command line flag to build debug versions
	@todo Add test name command line flag to build just certain test
	@todo Add list to list tests
	@todo Evaluate performance of compile and run of various compilers
	@todo Command line flag to print out values even when they are acceptable
*/
int main(int /*argc*/, const char * const /*argv*/[]) {
	struct HeaderCoverageResults {
		const char * const	header;
		uint32_t			coverage;
	};
	HeaderCoverageResults	expectedCoveragex86[]= {
		{"Address.h",			4},
		{"AddressIPv4.h",		10},
		{"AddressIPv6.h",		10},
		{"ArchiveFile.h",		146},
		{"AtomicInteger.h",		16},
		{"Bencode.h",			258},
		{"Buffer.h",			4},
		{"BufferAddress.h",		8},
		{"BufferManaged.h",		4},
		{"BufferString.h",		8},
		{"CompactNumber.h",		17},
		{"DateTime.h",			12},
		{"EnumSet.h",			191},
		{"Exception.h",			19},
		{"Execute.h",			7},
		{"File.h",				74},
		{"Hash.h",				52},
		{"Library.h",			113},
		{"Mutex.h",				15},
		{"POSIXErrno.h",		10},
		{"RWLock.h",			18},
		{"ReferenceCounted.h",	45},
		{"ReferencedString.h",	264},
		{"Socket.h",			19},
		{"SocketGeneric.h",		14},
		{"SocketServer.h",		14},
		{"Sqlite3Plus.h",		31},
		{"Thread.h",			30},
		{"Tracer.h",			0},
	};
	HeaderCoverageResults	expectedCoveragePPC[]= {
		{"Address.h",			4},
		{"AddressIPv4.h",		10},
		{"AddressIPv6.h",		10},
		{"ArchiveFile.h",		146},
		{"AtomicInteger.h",		16},
		{"Bencode.h",			258},
		{"Buffer.h",			4},
		{"BufferAddress.h",		8},
		{"BufferManaged.h",		4},
		{"BufferString.h",		8},
		{"CompactNumber.h",		17},
		{"DateTime.h",			12},
		{"EnumSet.h",			191},
		{"Exception.h",			19},
		{"Execute.h",			7},
		{"File.h",				74},
		{"Hash.h",				52},
		{"Library.h",			113},
		{"Mutex.h",				15},
		{"POSIXErrno.h",		10},
		{"RWLock.h",			18},
		{"ReferenceCounted.h",	45},
		{"ReferencedString.h",	264},
		{"Socket.h",			19},
		{"SocketGeneric.h",		14},
		{"SocketServer.h",		14},
		{"Sqlite3Plus.h",		31},
		{"Thread.h",			30},
		{"Tracer.h",			0},
	};
	struct TestExpectedResults {
		const char * const	name;
		const char * const	compilerresults;
	};
	TestExpectedResults testsx86[]= {
		{"ArchiveFile",			"clang++:138:0.047:1.485;"	"g++:138:0.025:2.083;"	"llvm-g++:138:0.024:1.872"},
		{"AtomicInteger",		"clang++:12:0.421:1.352;"	"g++:12:0.421:1.477;"	"llvm-g++:12:0.421:1.528"},
		{"Bencode",				"clang++:258:0.021:0.893;"	"g++:244:0.002:0.000;"	"llvm-g++:244:0.002:0.000"},
		{"CompactNumber",		"clang++:17:0.025:0.400;"	"g++:17:0.024:0.554;"	"llvm-g++:17:0.022:0.575"},
		{"DateTime",			"clang++:12:0.040:1.091;"	"g++:12:0.036:1.400;"	"llvm-g++:12:0.024:1.931"},
		{"EnumSet",				"clang++:191:0.020:0.814;"	"g++:191:0.019:0.989;"	"llvm-g++:191:0.021:0.987"},
		{"Exception",			"clang++:19:0.019:0.683;"	"g++:19:0.021:0.824;"	"llvm-g++:19:0.026:0.802"},
		{"Execute",				"clang++:7:10.949:3.746;"	"g++:7:10.949:3.823;"	"llvm-g++:7:10.949:4.332"},
		{"File",				"clang++:35:0.026:0.984;"	"g++:35:0.047:1.405;"	"llvm-g++:35:0.021:1.401"},
		{"Hash",				"clang++:52:0.022:0.745;"	"g++:52:0.019:0.819;"	"llvm-g++:52:0.020:0.826"},
		{"Library",				"clang++:113:0.248:2.110;"	"g++:113:0.248:2.250;"	"llvm-g++:113:0.248:2.216"},
		{"Mutex",				"clang++:15:0.039:0.770;"	"g++:15:0.037:0.933;"	"llvm-g++:15:0.037:0.900"},
		{"RWLock",				"clang++:18:10.440:21.146;"	"g++:18:10.421:21.668;"	"llvm-g++:18:10.437:20.627"},
		{"ReferenceCounted",	"clang++:45:3.061:3.047;"	"g++:45:3.061:3.284;"	"llvm-g++:45:3.061:3.216"},
		{"ReferencedString",	"clang++:251:0.041:1.114;"	"g++:251:0.028:0.932;"	"llvm-g++:251:0.028:0.905"},
		{"SocketServer",		"clang++:15:7.427:1.250;"	"g++:15:7.427:2.093;"	"llvm-g++:15:7.427:1.719"},
		{"Sqlite3Plus",			"clang++:31:0.324:1.324;"	"g++:31:0.324:2.474;"	"llvm-g++:31:0.324:1.925"},
		{"Thread",				"clang++:27:0.058:0.671;"	"g++:27:0.058:0.786;"	"llvm-g++:27:0.058:0.798"},
	};
	TestExpectedResults testsPPC[]= {
		{"ArchiveFile",			"clang++:138:0.024:1.403;"	"g++:138:0.023:2.083;"	"llvm-g++:138:0.024:1.872"},
		{"AtomicInteger",		"clang++:12:0.421:1.337;"	"g++:12:0.421:1.455;"	"llvm-g++:12:0.421:1.528"},
		{"Bencode",				"clang++:258:0.021:0.878;"	"g++:244:0.002:0.000;"	"llvm-g++:244:0.002:0.000"},
		{"CompactNumber",		"clang++:17:0.025:0.400;"	"g++:17:0.024:0.554;"	"llvm-g++:17:0.022:0.575"},
		{"DateTime",			"clang++:12:0.040:1.091;"	"g++:12:0.036:1.400;"	"llvm-g++:12:0.024:1.931"},
		{"EnumSet",				"clang++:191:0.020:0.814;"	"g++:191:0.019:0.989;"	"llvm-g++:191:0.021:0.987"},
		{"Exception",			"clang++:19:0.019:0.683;"	"g++:19:0.021:0.802;"	"llvm-g++:19:0.019:0.802"},
		{"Execute",				"clang++:7:10.949:3.746;"	"g++:7:10.949:3.726;"	"llvm-g++:7:10.949:4.332"},
		{"File",				"clang++:35:0.026:0.978;"	"g++:35:0.024:1.405;"	"llvm-g++:35:0.021:1.386"},
		{"Hash",				"clang++:52:0.022:0.700;"	"g++:52:0.019:0.813;"	"llvm-g++:52:0.020:0.826"},
		{"Library",				"clang++:113:0.248:2.110;"	"g++:113:0.248:2.250;"	"llvm-g++:113:0.248:2.216"},
		{"Mutex",				"clang++:15:0.039:0.770;"	"g++:15:0.036:0.895;"	"llvm-g++:15:0.037:0.892"},
		{"RWLock",				"clang++:18:10.440:20.991;"	"g++:18:10.421:21.668;"	"llvm-g++:18:10.437:20.592"},
		{"ReferenceCounted",	"clang++:45:3.061:2.849;"	"g++:45:3.061:3.036;"	"llvm-g++:45:3.061:2.949"},
		{"ReferencedString",	"clang++:251:0.041:1.114;"	"g++:251:0.028:0.932;"	"llvm-g++:251:0.028:0.905"},
		{"SocketServer",		"clang++:15:7.427:1.250;"	"g++:15:7.427:1.712;"	"llvm-g++:15:7.427:1.719"},
		{"Sqlite3Plus",			"clang++:31:0.324:1.324;"	"g++:31:0.324:1.907;"	"llvm-g++:31:0.324:1.925"},
		{"Thread",				"clang++:27:0.058:0.671;"	"g++:27:0.058:0.786;"	"llvm-g++:27:0.058:0.798"},
	};
	HeaderCoverageResults	*expectedCoverage= isIntel() ? &expectedCoveragex86[0] : &expectedCoveragePPC[0];
	uint32_t				expectedCoverageSize= isIntel()	? sizeof(expectedCoveragex86)/sizeof(expectedCoveragex86[0])
														: sizeof(expectedCoveragePPC)/sizeof(expectedCoveragePPC[0]);
	TestExpectedResults		*useResults= isIntel() ? &testsx86[0] : &testsPPC[0];
	uint32_t				useResultsSize= isIntel()	? sizeof(testsx86)/sizeof(testsx86[0])
														: sizeof(testsPPC)/sizeof(testsPPC[0]);

	try {
		std::string	results;
		StringList	headers;

		exec::execute("mkdir -p bin/tests bin/logs", results);
		if(results != "") {
			printf("WARNING: mkdir '%s'\n", results.c_str());
		}
		exec::execute("rm -Rf bin/logs/* bin/tests/*", results);
		if(results != "") {
			printf("WARNING: rm '%s'\n", results.c_str());
		}
		for(unsigned int test= 0; test < useResultsSize; ++test) {
			runTest(useResults[test].name, useResults[test].compilerresults);
		}
		printf("Examining overall coverage\n");
		exec::execute("ls *.h", results);
		split(results, '\n', headers);
		runNoResultsExpected("cat bin/logs/*_trace_run.log | grep /os/ | sort | uniq > bin/logs/all_trace.log", "Combining coverage");
		for(StringList::iterator header= headers.begin(); header != headers.end(); ++header) {
			uint32_t	coverage;
			bool		printed= false;

			coverage= runIntegerExpected("cat bin/logs/all_trace.log | grep /os/"+*header+": | wc -l");
			for(unsigned int headerIndex= 0; headerIndex < expectedCoverageSize; ++headerIndex) {
				if(expectedCoverage[headerIndex].header == *header) {
					if(expectedCoverage[headerIndex].coverage != coverage) {
						printf("\n%20s\tCoverage: %4d Expected: %4d\n", header->c_str(), coverage, expectedCoverage[headerIndex].coverage);
						printed= true;
					}
					break;
				}
			}
			if(!printed) {
				printf(".");
				fflush(stdout);
			}
		}
		printf("\n");
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
