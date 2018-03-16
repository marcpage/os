#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <ctype.h>
#include "os/Execute.h"
#include "os/DateTime.h"
#include "os/File.h"
#include "os/Exception.h"

struct Times {
	double		perfCompile, perfRun, traceCompile, traceRun;
	uint32_t	testedLines, warnings;
};
typedef std::string							String;
typedef std::vector<String>					StringList;
typedef std::map<String,String>				Dictionary;
typedef std::map<String,Times>				CompilerTimes;
typedef std::map<String,CompilerTimes>		TestCompilerTimes;
typedef std::map<uint32_t,bool>				LinesCovered;

const double		gTestTimeAllowancePercent= 5;
const double		gTestMinimumTimeInSeconds= 1;
const char * const	gCompilerFlags= "-I.. -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings -lsqlite3 -framework Carbon";
const uint32_t		gMinimumPercentCodeCoverage= 60;

TestCompilerTimes	gCompilerTimes;
Dictionary			gCompilerLocations;
bool				gDebugging= false;
bool				gVerbose= false;

String &stripEOL(String &s) {
	while( (s.size() > 0) && ((s[s.size()-1] == '\n') || (s[s.size()-1] == '\r')) ) {
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

long strtol(const String &s, int base=10) {
	char	*endptr = NULL;
	long	value = strtol(s.c_str(), &endptr, base);

	try {
		AssertMessageException( (NULL != endptr) && ('\0' == *endptr) );
	} catch(const std::exception &) {
		printf("strtol('%s', %d) -> endptr = %d value=%ld\n", s.c_str(), base, *endptr, value);
		throw;
	}
	return value;
}

int runIntegerExpected(const String &command) {
	String	results;

	return strtol(strip(exec::execute(command, results)));
}

double runNoResultsExpected(const String &command, const char * const action) {
	String		results;
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
	String		results;
	String		command;
	String		executableName;
	String		logName;
	String		runLogName;
	String		gcovLogName;
	dt::DateTime	start, end;
	double			compilePerfTime, compileCoverageTime, runPerfTime, runCoverageTime, totalTime;
	uint32_t		coverage;
	uint32_t		uncovered;
	uint32_t		percent_coverage;
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
		printf("%-18s %9s about %7.3fs", name.c_str(), compiler.c_str(), totalTimeInSeconds);
		fflush(stdout);
		executableName= name + '_' + compiler + "_performance";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";

		command+= " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+(gDebugging ? " -g " : "")+gCompilerFlags+" &> bin/logs/"+logName;
		compilePerfTime= runNoResultsExpected(command, "compile performance");

		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runPerfTime= runNoResultsExpected(command, "run performance");

		failures= runIntegerExpected("cat bin/logs/"+runLogName+" | grep FAIL | sort | uniq | wc -l");
		errors= runIntegerExpected("cat bin/logs/"+logName+" | grep error: | sort | uniq | wc -l");
		errors+= runIntegerExpected("cat bin/logs/"+logName+" | grep ld: | sort | uniq | wc -l");
		warnings= runIntegerExpected("cat bin/logs/"+logName+" | grep warning: | sort | uniq | wc -l");

		executableName= name + '_' + compiler + "_trace";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		gcovLogName= executableName + "_gcov.log";
		command= gCompilerLocations[compiler]
					+ " -o bin/tests/"+executableName+" tests/"+name+"_test.cpp "
					+(gDebugging ? " -g " : "")+gCompilerFlags+" -D__Tracer_h__ -fprofile-arcs -ftest-coverage &> bin/logs/"+logName;
		compileCoverageTime= runNoResultsExpected(command, "compile trace");
		command= "bin/tests/"+executableName+" &> bin/logs/"+runLogName;
		runCoverageTime= runNoResultsExpected(command, "run trace");

		exec::execute("mkdir -p bin/coverage/"+executableName, results);
		if(results != "") {
			printf("WARNING: mkdir '%s'\n", results.c_str());
		}
		exec::execute("gcov "+name+"_test.cpp"+" &> bin/logs/"+gcovLogName, results);
		if(results != "") {
			printf("WARNING: gcov '%s'\n", results.c_str());
		}
		exec::execute("mv *.gcov *.gcno *.gcda bin/coverage/"+executableName+"/", results);
		if(results != "") {
			printf("WARNING: mv '%s'\n", results.c_str());
		}

		coverage= runIntegerExpected("cat bin/coverage/"+executableName+"/"+name+".h.gcov | grep -E '[0-9]+:\\s+[0-9]+:' | wc -l");
		uncovered= runIntegerExpected("cat bin/coverage/"+executableName+"/"+name+".h.gcov | grep -E '#+:\\s+[0-9]+:' | wc -l");
		percent_coverage= 100 * coverage / (coverage + uncovered);
		printf("\t%3d%% coverage\n", percent_coverage);

		if(runPerfTime < gTestMinimumTimeInSeconds * (1.0 + gTestTimeAllowancePercent/100.0) ) {
			printf("\tTest is too short, run it %0.1f times\n", 1.0 / runPerfTime);
		}
		if(failures > 0) {
			printf("\t%d Test Failures\n", failures);
		}
		if(errors > 0) {
			printf("\t%d Compile Errors\n", errors);
		}
		if(warnings > 0) {
			printf("\t%d Compile Warnings\n", warnings);
		}
		if ((gVerbose && (uncovered > 0)) || (percent_coverage < gMinimumPercentCodeCoverage)) {
			printf("\twarning: %d lines untested (%d tested) %d%%\n", uncovered, coverage, percent_coverage);
			exec::execute("cat bin/coverage/"+executableName+"/"+name+".h.gcov | grep -E '#+:\\s+[0-9]+:'", results);
			printf("%s\n", results.c_str());
		}
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
		String	compiler;
		uint32_t	testedLines;
		double		durationInSeconds;
		double		totalTimeInSeconds;

		split(*compilerInfo, ':', values);
		compiler= values[0];
		testedLines= strtol(strip(values[1]));
		durationInSeconds= atof(values[2].c_str());
		totalTimeInSeconds= atof(values[3].c_str());
		runTest(name, compiler, testedLines, durationInSeconds, totalTimeInSeconds);
	}
}


void loadExpectations(Dictionary &headerCoverage, Dictionary &testMetrics, StringList &testOrder) {
	const char * const	processor= "x64";
	io::File			expectations(String("tests/test_")+processor+".txt",
										io::File::Text, io::File::ReadOnly);
	String			line;
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

			for(String::size_type c= 0; c < line.size(); ++c) {
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

void findFileCoverage(const String &file, uint32_t &covered, uint32_t &uncovered) {
	String		results;
	StringList		lines;
	StringList		parts;
	LinesCovered	coveredLines;

	exec::execute("cat bin/coverage/*/"+file+".gcov | grep -v -E -e '-:\\s+[0-9]+:' | cut -d: -f1-2", results);
	split(results, '\n', lines);
	for (StringList::iterator line = lines.begin(); line != lines.end(); ++line) {
		split(*line, ':', parts);

		const bool lineRun = strip(parts[0]).substr(0,1) != "#";
		int lineNumber = strtol(strip(parts[1]));
		
		if (!coveredLines[lineNumber] && lineRun) {
			coveredLines[lineNumber] = true;
		}
	}
	covered = 0;
	uncovered = 0;
	for (LinesCovered::iterator i = coveredLines.begin(); i != coveredLines.end(); ++i) {
		if (i->second) {
			covered += 1;
		} else {
			uncovered += 1;
		}
	}
}

/**
	@todo Evaluate performance of compile and run of various compilers
*/
int main(int argc, const char * const argv[]) {
	StringList				testsToRun;
	Dictionary				headerCoverage, testMetrics;
	
	loadExpectations(headerCoverage, testMetrics, testsToRun);
	for(int arg= 1; arg < argc; ++arg) {
		if(String("debug") == argv[arg]) {
			gDebugging= true;
		} else if(String("list") == argv[arg]) {
			testsToRun.clear();
			for(StringList::iterator test= testsToRun.begin(); test != testsToRun.end(); ++test) {
				printf("%s\n", test->c_str());
			}
		} else if(String("verbose") == argv[arg]) {
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
		String	results;
		StringList	headers;

		exec::execute("mkdir -p bin/tests bin/logs", results);
		if(results != "") {
			printf("WARNING: mkdir '%s'\n", results.c_str());
		}
		exec::execute("rm -Rf *.gcov *.gcno *.gcda bin/coverage/* bin/logs/* bin/tests/*", results);
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
			for(StringList::iterator header= headers.begin(); header != headers.end(); ++header) {
				uint32_t	coverage;
				uint32_t	uncovered;
				bool		found= headerCoverage.count(*header) > 0;
				int			value= found ? strtol(strip(headerCoverage[*header])) : 0;
				
				findFileCoverage(*header, coverage, uncovered);
				if ( (100 * coverage / (coverage + uncovered) < gMinimumPercentCodeCoverage) || (gVerbose && (uncovered > 0))) {
					printf("%s coverage low %d%%\n", header->c_str(), 100 * coverage / (coverage + uncovered));
					exec::execute("cat bin/coverage/*/"+*header+".gcov | grep -E '#+:\\s+[0-9]+:' | cut -d: -f2- | sort | uniq", results);
					printf("%s\n", results.c_str());
				}
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
