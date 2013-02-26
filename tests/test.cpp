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

const double		gTestTimeAllowancePercent= 2;
const double		gTestMinimumTimeInSeconds= 1;
const char * const	gCompilerFlags= "-I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -lsqlite3 -framework Carbon";

TestCompilerTimes	gCompilerTimes;
Dictionary			gCompilerLocations;
bool				gDebugging= false;
bool				gVerbose= false;

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
	uint32_t		warnings, errors, failures;

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
					+(gDebugging ? " -g " : "")+gCompilerFlags+" &> bin/logs/"+logName;
		compilePerfTime= runNoResultsExpected(command, "compile performance");
		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runPerfTime= runNoResultsExpected(command, "run performance");

		if(runPerfTime < gTestMinimumTimeInSeconds * (1.0 + gTestTimeAllowancePercent/100.0) ) {
			printf("\tTest is too short, run it %0.1f times\n", 1.0 / runPerfTime);
		}

		failures= runIntegerExpected("cat bin/logs/"+runLogName+" | grep FAIL | sort | uniq | wc -l");
		if(failures > 0) {
			printf("\t%d Test Failures\n", failures);
		}

		errors= runIntegerExpected("cat bin/logs/"+logName+" | grep error: | sort | uniq | wc -l");
		errors+= runIntegerExpected("cat bin/logs/"+logName+" | grep ld: | sort | uniq | wc -l");
		if(errors > 0) {
			printf("\t%d Compile Errors\n", errors);
		}

		warnings= runIntegerExpected("cat bin/logs/"+logName+" | grep warning: | sort | uniq | wc -l");
		if(warnings > 0) {
			printf("\t%d Compile Warnings\n", warnings);
		}

		executableName= name + '_' + compiler + "_trace";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		command= gCompilerLocations[compiler]
					+ " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+(gDebugging ? " -g " : "")+gCompilerFlags+" -include Tracer.h &> bin/logs/"+logName;
		compileCoverageTime= runNoResultsExpected(command, "compile trace");
		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runCoverageTime= runNoResultsExpected(command, "run trace");

		coverage= runIntegerExpected("cat bin/logs/"+runLogName+" | grep "+name+"\\\\.h | sort | uniq | wc -l");
		if(gVerbose || (coverage != testedLines) ) {
			printf("\tTested Lines: %d Expected %d\n", coverage, testedLines);
		}
		if(gVerbose || (runPerfTime > durationInSeconds * (1 + gTestTimeAllowancePercent/100) ) ) {
			printf("\tTest took %0.3fs seconds, expected %0.3fs seconds\n", runPerfTime, durationInSeconds);
		}
		totalTime= compilePerfTime + compileCoverageTime + runPerfTime + runCoverageTime;
		if(gVerbose || (totalTime > totalTimeInSeconds) ) {
			printf("\tBuild/Test took %0.3fs seconds, expected %0.3fs seconds\n", totalTime, totalTimeInSeconds);
		}
		if(gVerbose) {
			printf("Coverage %d (expected %d) Compile: %0.3fs Run: %0.3fs (expected %0.3fs) Trace Compile: %0.3fs Trace Run: %0.3fs\n",
				coverage, testedLines,
				compilePerfTime, runPerfTime, durationInSeconds, compileCoverageTime, runCoverageTime
			);
		}
		gCompilerTimes[name][compiler].perfCompile= compilePerfTime;
		gCompilerTimes[name][compiler].perfRun= runPerfTime;
		gCompilerTimes[name][compiler].traceCompile= compileCoverageTime;
		gCompilerTimes[name][compiler].traceRun= runCoverageTime;
		gCompilerTimes[name][compiler].testedLines= coverage;
		gCompilerTimes[name][compiler].warnings= warnings;
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
	@todo Evaluate performance of compile and run of various compilers
	@todo Load tables from text file instead of compiled in
*/
int main(int argc, const char * const argv[]) {
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
		{"ArchiveFile.h",		138},
		{"AtomicInteger.h",		14},
		{"Bencode.h",			255},
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
		{"ReferencedString.h",	251},
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
		{"Bencode",				"clang++:258:0.021:0.893;"	"g++:244:0.002:0.000;"	"llvm-g++:244:0.002:0.000"},
		{"SocketServer",		"clang++:15:7.427:1.250;"	"g++:15:7.427:2.093;"	"llvm-g++:15:7.427:4.685"},
		{"ArchiveFile",			"clang++:138:0.047:1.485;"	"g++:138:0.032:2.083;"	"llvm-g++:138:0.024:1.872"},
		{"AtomicInteger",		"clang++:12:2.449:8.042;"	"g++:12:2.450:7.359;"	"llvm-g++:12:3.134:7.683"},
		{"CompactNumber",		"clang++:17:1.253:1.628;"	"g++:17:1.283:1.744;"	"llvm-g++:17:1.270:1.736"},
		{"DateTime",			"clang++:12:2.725:3.664;"	"g++:12:1.225:2.572;"	"llvm-g++:12:1.433:2.737"},
		{"EnumSet",				"clang++:191:1.397:2.514;"	"g++:191:1.091:2.043;"	"llvm-g++:191:1.152:2.074"},
		{"Exception",			"clang++:19:2.930:3.558;"	"g++:19:2.833:3.600;"	"llvm-g++:19:2.849:3.622"},
		{"Execute",				"clang++:7:10.949:3.794;"	"g++:7:10.949:3.896;"	"llvm-g++:7:10.949:4.332"},
		{"File",				"clang++:35:11.083:12.009;"	"g++:35:8.179:9.496;"	"llvm-g++:35:7.648:8.975"},
		{"Hash",				"clang++:52:1.365:2.482;"	"g++:52:1.364:2.428;"	"llvm-g++:52:1.347:2.140"},
		{"Library",				"clang++:113:5.606:7.572;"	"g++:113:6.060:8.181;"	"llvm-g++:113:5.551:7.714"},
		{"Mutex",				"clang++:15:3.603:4.372;"	"g++:15:3.302:4.260;"	"llvm-g++:15:3.209:4.094"},
		{"RWLock",				"clang++:18:10.440:21.146;"	"g++:18:10.421:21.668;"	"llvm-g++:18:10.437:21.151"},
		{"ReferenceCounted",	"clang++:45:3.061:7.780;"	"g++:45:3.061:7.969;"	"llvm-g++:45:3.061:8.030"},
		{"ReferencedString",	"clang++:251:1.813:2.767;"	"g++:251:1.625:2.498;"	"llvm-g++:251:1.635:2.506"},
		{"Sqlite3Plus",			"clang++:31:0.324:1.324;"	"g++:31:0.324:2.474;"	"llvm-g++:31:0.324:1.925"},
		{"Thread",				"clang++:27:1.649:4.966;"	"g++:27:1.669:5.469;"	"llvm-g++:27:1.653:5.178"},
	};
	TestExpectedResults testsPPC[]= {
		{"ArchiveFile",			"clang++:138:1.134:23.888;"	"g++:138:0.352:25.298;"	"llvm-g++:138:0.024:1.872"},
		{"AtomicInteger",		"clang++:12:14.388:59.800;"	"g++:12:14.496:39.122;"	"llvm-g++:12:0.421:1.528"},
		{"Bencode",				"clang++:258:0.021:0.878;"	"g++:244:0.002:0.000;"	"llvm-g++:244:0.002:0.000"},
		{"CompactNumber",		"clang++:17:40.032:59.677;"	"g++:17:43.080:57.276;"	"llvm-g++:17:0.022:0.575"},
		{"DateTime",			"clang++:12:32.487:71.275;"	"g++:12:38.545:78.598;"	"llvm-g++:12:0.024:1.931"},
		{"EnumSet",				"clang++:191:32.206:59.828;""g++:191:23.410:44.149;""llvm-g++:191:0.021:0.987"},
		{"Exception",			"clang++:19:104.078:123.402;g++:19:94.703:114.685;"	"llvm-g++:19:0.019:0.802"},
		{"Execute",				"clang++:7:10.949:55.717;"	"g++:7:10.949:29.929;"	"llvm-g++:7:10.949:4.332"},
		{"File",				"clang++:35:4.214:39.869;"	"g++:35:16.517:58.532;"	"llvm-g++:35:0.021:1.386"},
		{"Hash",				"clang++:52:22.415:50.014;"	"g++:52:17.206:50.014;"	"llvm-g++:52:0.020:0.826"},
		{"Library",				"clang++:113:56.031:89.068;""g++:113:51.360:89.068;""llvm-g++:113:0.248:2.216"},
		{"Mutex",				"clang++:15:33.332:49.005;"	"g++:15:43.793:66.504;"	"llvm-g++:15:0.037:0.892"},
		{"RWLock",				"clang++:18:10.440:28.759;"	"g++:18:10.421:22.759;"	"llvm-g++:18:10.437:20.592"},
		{"ReferenceCounted",	"clang++:45:35.111:72.466;"	"g++:45:34.677:77.064;"	"llvm-g++:45:3.061:2.949"},
		{"ReferencedString",	"clang++:251:11.681:33.729;""g++:251:11.444:23.692;""llvm-g++:251:0.028:0.905"},
		{"SocketServer",		"clang++:15:7.427:1.250;"	"g++:15:7.427:1.712;"	"llvm-g++:15:7.427:1.719"},
		{"Sqlite3Plus",			"clang++:31:0.356:21.786;"	"g++:31:0.324:20.688;"	"llvm-g++:31:0.324:1.925"},
		{"Thread",				"clang++:27:5.439:36.546;"	"g++:27:17.898:173.261;"	"llvm-g++:27:0.058:0.798"},
	};
	HeaderCoverageResults	*expectedCoverage= isIntel() ? &expectedCoveragex86[0] : &expectedCoveragePPC[0];
	uint32_t				expectedCoverageSize= isIntel()	? sizeof(expectedCoveragex86)/sizeof(expectedCoveragex86[0])
														: sizeof(expectedCoveragePPC)/sizeof(expectedCoveragePPC[0]);
	TestExpectedResults		*useResults= isIntel() ? &testsx86[0] : &testsPPC[0];
	uint32_t				useResultsSize= isIntel()	? sizeof(testsx86)/sizeof(testsx86[0])
														: sizeof(testsPPC)/sizeof(testsPPC[0]);
	StringList				testsToRun;

	for(unsigned int test= 0; test < useResultsSize; ++test) {
		testsToRun.push_back(useResults[test].name);
	}
	for(int arg= 1; arg < argc; ++arg) {
		if(std::string("debug") == argv[arg]) {
			gDebugging= true;
		} else if(std::string("list") == argv[arg]) {
			testsToRun.clear();
			for(unsigned int test= 0; test < useResultsSize; ++test) {
				printf("%s\n", useResults[test].name);
			}
		} else if(std::string("verbose") == argv[arg]) {
			gVerbose= true;
		} else {
			bool	found= false;

			for(unsigned int test= 0; test < useResultsSize; ++test) {
				if(std::string(argv[arg]) == useResults[test].name) {
					found= true;
					break;
				}
			}
			if(found) {
				if(testsToRun.size() > static_cast<unsigned int>(arg)) {
					testsToRun.clear();
				}
				testsToRun.push_back(argv[arg]);
			} else {
				printf("Test not found: %s\n", argv[arg]);
			}
		}
	}
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
		for(StringList::iterator test= testsToRun.begin(); test != testsToRun.end(); ++test) {
			for(unsigned int testIndex= 0; testIndex < useResultsSize; ++testIndex) {
				if(useResults[testIndex].name == *test) {
					runTest(useResults[testIndex].name, useResults[testIndex].compilerresults);
				}
			}
		}
		if(testsToRun.size() > 0) {
			printf("Examining overall coverage ...\n");
			exec::execute("ls *.h", results);
			split(results, '\n', headers);
			runNoResultsExpected("cat bin/logs/*_trace_run.log | grep /os/ | sort | uniq > bin/logs/all_trace.log", "Combining coverage");
			for(StringList::iterator header= headers.begin(); header != headers.end(); ++header) {
				uint32_t	coverage;
				bool		found= false;

				coverage= runIntegerExpected("cat bin/logs/all_trace.log | grep /os/"+*header+": | wc -l");
				for(unsigned int headerIndex= 0; headerIndex < expectedCoverageSize; ++headerIndex) {
					if(expectedCoverage[headerIndex].header == *header) {
						found= true;
						if(expectedCoverage[headerIndex].coverage != coverage) {
							printf("%20s\tCoverage: %4d Expected: %4d\n", header->c_str(), coverage, expectedCoverage[headerIndex].coverage);
						}
						break;
					}
				}
				if(!found) {
					printf("WARNING: No data for header %s with coverage %d\n", header->c_str(), coverage);
				}
			}
		}
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
