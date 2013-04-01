#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <ctype.h>
#include "os/Execute.h"
#include "os/DateTime.h"
#include "os/File.h"

struct Times {
	double		perfCompile, perfRun, traceCompile, traceRun;
	uint32_t	testedLines, warnings;
};
typedef std::string								String;
typedef std::vector<String>						StringList;
typedef std::map<std::string,std::string>		Dictionary;
typedef std::map<std::string,Times>				CompilerTimes;
typedef std::map<std::string,CompilerTimes>		TestCompilerTimes;

const double		gTestTimeAllowancePercent= 5;
const double		gTestMinimumTimeInSeconds= 1;
const char * const	gCompilerFlags= "-I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -lsqlite3 -framework Carbon";

TestCompilerTimes	gCompilerTimes;
Dictionary			gCompilerLocations;
bool				gDebugging= false;
bool				gVerbose= false;

String &stripEOL(String &s) {
	while( (s.size() > 0) && (s[s.size()-1] == '\n') ) {
		s.erase(s.size()-1);
	}
	return s;
}

String &strip(String &s) {
	while( (s.size() > 0) && isspace(s[0])) {
		s.erase(0, 1);
	}
	while( (s.size() > 0) && isspace(s[s.size() - 1])) {
		s.erase(s.size() - 1);
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
	bool			displayNewLine= false;

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

		coverage= runIntegerExpected("cat bin/logs/"+runLogName+" | grep ../os/"+name+"\\\\.h: | sort | uniq | wc -l");
		if( (coverage != testedLines) ) {
			printf("\tTested Lines: %d Expected %d\n", coverage, testedLines);
			displayNewLine= true;
		}
		if( (runPerfTime > durationInSeconds * (1 + gTestTimeAllowancePercent/100) ) ) {
			printf("\tTest took %0.3fs seconds, expected %0.3fs seconds\n", runPerfTime+0.000999, durationInSeconds);
			displayNewLine= true;
		}
		totalTime= compilePerfTime + compileCoverageTime + runPerfTime + runCoverageTime;
		if( (totalTime > totalTimeInSeconds) ) {
			printf("\tBuild/Test took %0.3fs seconds, expected %0.3fs seconds\n", totalTime+0.000999, totalTimeInSeconds);
			displayNewLine= true;
		}
		if(displayNewLine) {
			printf("\t%s:%d:%0.3f:%0.3f\n",
				compiler.c_str(), coverage,
				runPerfTime > durationInSeconds ? runPerfTime+0.000999 : durationInSeconds,
				totalTime > totalTimeInSeconds ? totalTime+0.000999 : totalTimeInSeconds
			);
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

void loadExpectations(Dictionary &headerCoverage, Dictionary &testMetrics, StringList &testOrder) {
	const char * const	processor= isIntel() ? "i386" : "ppc";
	io::File			expectations(std::string("tests/test_")+processor+".txt",
										io::File::Text, io::File::ReadOnly);
	std::string			line;
	Dictionary			*current= NULL;
	bool				eof;

	do	{
		expectations.readline(line);
		eof= (line.size() == 0);
		if(strip(line).size() == 0) {
			// ignore blank lines
		} else if(line[0] == '-') {
			if(line == "-header") {
				current= &headerCoverage;
			} else if(line == "-test") {
				current= &testMetrics;
			} else {
				current= NULL;
			}
		} else if(NULL != current) {
			StringList	parts;
			String		key, value;
			String		separator= "";

			for(std::string::size_type c= 0; c < line.size(); ++c) {
				if(line[c] == ' ') {
					line[c]= '\t';
				}
			}
			split(line, '\t', parts);
			key= parts[0];
			if(current == &testMetrics) {
				testOrder.push_back(key);
			}
			parts.erase(parts.begin());
			while(parts.size() > 0) {
				if(parts[0].size() > 0) {
					value+= separator + parts[0];
					separator= ';';
				}
				parts.erase(parts.begin());
			}
			(*current)[key]= value;
		}
	} while(!eof);
}

/**
	@todo Evaluate performance of compile and run of various compilers
*/
int main(int argc, const char * const argv[]) {
	StringList				testsToRun;
	Dictionary				headerCoverage, testMetrics;

	loadExpectations(headerCoverage, testMetrics, testsToRun);
	for(int arg= 1; arg < argc; ++arg) {
		if(std::string("debug") == argv[arg]) {
			gDebugging= true;
		} else if(std::string("list") == argv[arg]) {
			testsToRun.clear();
			for(StringList::iterator test= testsToRun.begin(); test != testsToRun.end(); ++test) {
				printf("%s\n", test->c_str());
			}
		} else if(std::string("verbose") == argv[arg]) {
			gVerbose= true;
		} else {
			bool	found= testMetrics.count(argv[arg]) > 0;

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
			runTest(*test, testMetrics[*test]);
		}
		if(testsToRun.size() > 0) {
			printf("Examining overall coverage ...\n");
			exec::execute("ls *.h", results);
			split(results, '\n', headers);
			runNoResultsExpected("cat bin/logs/*_trace_run.log | grep /os/ | sort | uniq > bin/logs/all_trace.log", "Combining coverage");
			for(StringList::iterator header= headers.begin(); header != headers.end(); ++header) {
				uint32_t	coverage;
				bool		found= headerCoverage.count(*header) > 0;
				int			value= found ? atoi(headerCoverage[*header].c_str()) : 0;

				coverage= runIntegerExpected("cat bin/logs/all_trace.log | grep ../os/"+*header+": | wc -l");
				if(found) {
					if(value != static_cast<int>(coverage)) {
						printf("%20s\tCoverage: %4d Expected: %4d\n", header->c_str(), coverage, value);
					}
				} else {
					printf("WARNING: No data for header %s with coverage %d\n", header->c_str(), coverage);
				}
			}
		}
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
