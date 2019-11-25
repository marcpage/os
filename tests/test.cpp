#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <ctype.h>
#include "os/Execute.h"
#include "os/DateTime.h"
#include "os/File.h"
#include "os/Exception.h"
#include "os/Path.h"
#include "os/Sqlite3Plus.h"
#include "os/Hash.h"
#include "os/Environment.h"
#include "os/Statistics.h"

// select name, compiler, options, count(*) as count, avg(run_time) as average_run_time, min(run_time) as min_run_time, max(run_time) as max_run_time, avg(100*lines_run/code_lines) as average_coverage, min(100*lines_run/code_lines) as min_coverage, max(100*lines_run/code_lines) as max_coverage from run group by name, test_hash, header_hash, compiler, options;
// select name, compiler, options, count(*) as count, avg(run_time) as average_run_time, min(run_time) as min_run_time, max(run_time) as max_run_time, avg(100*lines_run/code_lines) as average_coverage, min(100*lines_run/code_lines) as min_coverage, max(100*lines_run/code_lines) as max_coverage from run group by name, source_identifier, compiler, options;

struct Times {
	double		perfCompile, perfRun, traceCompile, traceRun;
	uint32_t	testedLines, warnings;
};
typedef std::string							String;
typedef std::vector<String>					StringList;
typedef std::map<String,String>				Dictionary;

const double		gTestTimeAllowancePercent= 5;
const double		gTestMinimumTimeInSeconds= 1;
const char * const	gOpensllFlags= "-lcrypto";
const char * const	gCompilerFlags= "-I.. -MMD -std=c++11"
										" -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings"
										" -lz -lsqlite3 -framework CoreFoundation";
const uint32_t		gMinimumPercentCodeCoverage= 70;
const String		gCompilerList= "clang++,g++,llvm-g++";
Dictionary			gCompilerLocations;
bool				gDebugging= false;
bool				gVerbose= false;

/*
         foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27
*/
#define ErrorTextFormatStart "\033[41;1m"
#define WarningTextFormatStart "\033[43;30;1m"
#define GoodTextFormatStart "\033[42;30;1m"
#define GreatTextFormatStart "\033[42;34;1m"
#define BoldTextFormatStart "\033[1m"
#define ClearTextFormat "\033[0m"

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

int runIntegerExpected(const String &command) {
	String	results;

	return std::stoi(strip(exec::execute(command, results)));
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

String fileContents(const String &path) {
	try {
		return io::Path(path).contents();
	} catch(const posix::err::Errno &) {
		fprintf(stderr, "Error: Unable to read '%s'\n", path.c_str());
		throw;
	}
}

String &hashFile(const String &path, String &buffer) {
	return hash::sha256(fileContents(path)).hex(buffer);
}

int mystoi(const std::string &number) {
	try {
		return std::stoi(number);
	} catch(const std::exception&) {}
	return 0;
}

double mystod(const std::string &number) {
	try {
		return std::stod(number);
	} catch(const std::exception&) {}
	return 0.0;
}

void getHeaderStats(Sqlite3::DB &db, const String &name, const String &headerHash, const String &options, int &linesRun, int &linesNotRun, const std::string &testNames, int &bestPercent) {
	Sqlite3::DB::Results	results;

	db.exec("SELECT name,lines_run,code_lines,timestamp FROM header WHERE name LIKE '" + name + "' AND tests LIKE '" + testNames + "' AND options LIKE '"+options+"' ORDER BY timestamp DESC;", &results);
	if (results.size() > 0) {
		linesRun= mystoi(results[0]["lines_run"]);
		linesNotRun= mystoi(results[0]["code_lines"]) - linesRun;
	} else {
		linesRun= 0;
		linesNotRun= 0;
	}
	db.exec("SELECT MAX(100 * lines_run / code_lines) AS coverage FROM header WHERE name LIKE '"+name+"' AND tests like '"+testNames+"' AND hash LIKE '"+headerHash+"';", &results);
	if ( (results.size() > 0) && (results[0]["coverage"].length() > 0) ) {
		bestPercent= mystoi(results[0]["coverage"]);
	} else {
		bestPercent= 0;
	}
}

void updateHeaderStats(Sqlite3::DB &db, const String &name, const String &options, int linesRun, int linesNotRun, const std::string &testNames) {
	Sqlite3::DB::Row	row;
	String				buffer;

	row["name"]= name;
	row["options"]= options;
	row["hash"]= hashFile(name, buffer);
	row["lines_run"]= Sqlite3::toString(linesRun, buffer);
	row["code_lines"]= Sqlite3::toString(linesRun + linesNotRun, buffer);
	row["tests"]= testNames;
	row["timestamp"]= dt::DateTime().format("%Y/%m/%d %H:%M:%S", buffer);
	db.addRow("header", row);
}

void updateRunStats(Sqlite3::DB &db, const String &name, const String &headerHash, const String &testHash, const String &compiler, int linesRun, int linesNotRun, double traceBuildTime, double traceRunTime, double buildTime, double runTime, const String &options, const String &sourceIdentifier) {
	Sqlite3::DB::Row	row;
	String				buffer;

	row["name"]= name;
	row["header_hash"]= headerHash;
	row["test_hash"]= testHash;
	row["compiler"]= compiler;
	row["lines_run"]= Sqlite3::toString(linesRun, buffer);
	row["code_lines"]= Sqlite3::toString(linesRun + linesNotRun, buffer);
	row["trace_build_time"]= Sqlite3::toString(traceBuildTime, buffer);
	row["trace_run_time"]= Sqlite3::toString(traceRunTime, buffer);
	row["build_time"]= Sqlite3::toString(buildTime, buffer);
	row["run_time"]= Sqlite3::toString(runTime, buffer);
	row["options"]= options;
	row["source_identifier"]= sourceIdentifier;
	row["timestamp"]= dt::DateTime().format("%Y/%m/%d %H:%M:%S", buffer);
	db.addRow("run", row);
}

String::size_type skip(const String &text, String::size_type &index, bool whitespace) {
	while ( (index < text.length()) && ((::isspace(text[index]) != 0) == whitespace) ) {++index;}
	return index;
}

void removeLineEndingEscapes(String &text) {
	String::size_type	backslash= 0;

	while (backslash != String::npos) {
		if (String::npos != (backslash= text.find("\\\n"))) {
			text.replace(backslash, 2, 1, ' ');
		} else if (String::npos != (backslash= text.find("\\\r\n"))) {
			text.replace(backslash, 3, 1, ' ');
		} else if (String::npos != (backslash= text.find("\\\r"))) {
			text.replace(backslash, 2, 1, ' ');
		}
	}
}

String sourceIdentifier(const String &dependenciesPath) {
	String				contents= fileContents(dependenciesPath);
	String::size_type	startPos= contents.find(':');
	String				identifier("");
	String				hashBuffer;
	String				prefix("");

	removeLineEndingEscapes(contents);
	if (String::npos != startPos) {
		String::size_type	endPos;
		String				path;

		++startPos;
		while (startPos < contents.length()) {
			skip(contents, startPos, true);
			endPos= startPos;
			skip(contents, endPos, false);
			strip(path.assign(contents, startPos, endPos - startPos));
			if (path.length() > 0) {
				identifier+= prefix+path+":"+hashFile(path, hashBuffer);
				prefix= ",";
			}
			startPos= endPos;
		}
	}
	return identifier;
}

typedef std::pair<math::List, math::List> 		BuildAndRunTimesList;
typedef std::map<String, BuildAndRunTimesList>	CompilerBuildAndRunTimesList;
void updateCompilerTimesIfNeeded(CompilerBuildAndRunTimesList &compilerTimes, CompilerBuildAndRunTimesList &compilerDifferences,
								const String &compiler, const String &source, String &currentCompiler, String &currentSource,
								double &buildSum, double &runSum, double &count) {
	const bool sourceChanged = currentSource != source;
	const bool compilerChanged = currentCompiler != compiler;

	if ( sourceChanged || compilerChanged ) {
		if (count > 0) {
			compilerTimes[currentCompiler].first.push_back(buildSum / count);
			compilerTimes[currentCompiler].second.push_back(runSum / count);
		}

		if ( sourceChanged && (compilerTimes.size() > 0) ) {
			std::map<String, double> compilerAverages;
			double minBuild = std::numeric_limits<double>::max();
			double minRun = std::numeric_limits<double>::max();

			for (auto compilerAndTimes = compilerTimes.begin(); compilerAndTimes != compilerTimes.end(); ++compilerAndTimes) {
				double buildTime = math::mean(compilerAndTimes->second.first);
				double runTime = math::mean(compilerAndTimes->second.second);

				if (buildTime < minBuild) {
					minBuild = buildTime;
				}
				if (runTime < minRun) {
					minRun = runTime;
				}
			}
			for (auto compilerAndTimes = compilerTimes.begin(); compilerAndTimes != compilerTimes.end(); ++compilerAndTimes) {
				double buildTime = math::mean(compilerAndTimes->second.first);
				double runTime = math::mean(compilerAndTimes->second.second);

				printf("%s build %7.10f%% %7.10fs min %7.10fs\n", compilerAndTimes->first.c_str(), 100 * buildTime / minBuild, buildTime, minBuild);
				printf("%s run   %7.10f%% %7.10fs min %7.10fs\n", compilerAndTimes->first.c_str(), 100 * runTime / minRun, runTime, minRun);
				compilerDifferences[compilerAndTimes->first].first.push_back(buildTime / minBuild);
				compilerDifferences[compilerAndTimes->first].second.push_back(runTime / minRun);
			}
		}

		currentSource = source;
		currentCompiler = compiler;
		runSum = 0;
		buildSum = 0;
		count = 0;
		compilerTimes.clear();
	}
}

void dumpCompilerStats(Sqlite3::DB &db) {
	Sqlite3::DB::Results	results;
	CompilerBuildAndRunTimesList	compilerTimes, compilerDifferences;
	String	currentSource, currentCompiler;
	double	runSum, count = 0, buildSum;

	db.exec("SELECT "
				"name,run_time,build_time,compiler,source_identifier "
			"FROM run "
			"GROUP BY source_identifier,compiler"
			";", &results);

	for (auto row = results.begin(); row != results.end(); ++row) {
		updateCompilerTimesIfNeeded(compilerTimes, compilerDifferences,
									(*row)["compiler"], (*row)["source_identifier"], currentCompiler, currentSource,
									buildSum, runSum, count);
		buildSum += mystod((*row)["build_time"]);
		runSum += mystod((*row)["run_time"]);
		count += 1;
	}

	updateCompilerTimesIfNeeded(compilerTimes, compilerDifferences,
								"", "",
								currentCompiler, currentSource,
								buildSum, runSum, count);

	printf("Build Times\n");
	for (auto compilerAndLists = compilerDifferences.begin(); compilerAndLists != compilerDifferences.end(); ++compilerAndLists) {
		double mean = math::mean(compilerAndLists->second.first);
		double stddev = math::stddev(compilerAndLists->second.first);

		printf("\t""%s %7.10fs +/- %7.10fs\n", compilerAndLists->first.c_str(), mean, stddev);
	}

	printf("Run Times\n");
	for (auto compilerAndLists = compilerDifferences.begin(); compilerAndLists != compilerDifferences.end(); ++compilerAndLists) {
		double mean = math::mean(compilerAndLists->second.second);
		double stddev = math::stddev(compilerAndLists->second.second);

		printf("\t""%s %7.10fs +/- %7.10fs\n", compilerAndLists->first.c_str(), mean, stddev);
	}
}

void getTestStats(const String &name, const String &options, const String &headerHash, const String &testHash, uint32_t &testedLines, double &durationInSeconds, double &meanInSeconds, double &timeStddev, double &totalTimeInSeconds, double &slowTimeInSeconds, Sqlite3::DB &db) {
	Sqlite3::DB::Results	results;
	math::List				times;

	db.exec("SELECT "
				"MAX(lines_run) AS testedLines, "
				"MAX(run_time) AS durationInSeconds, "
				"MAX(trace_build_time + trace_run_time + build_time + run_time) AS totalTimeInSeconds, "
				"AVG(run_time) AS averageTime "
			"FROM run WHERE "
				"name LIKE '" + name + "' "
				"AND options LIKE '"+options+"' "
				"AND header_hash LIKE '"+headerHash+"' "
				"AND test_hash LIKE '"+testHash+"';", &results);

	if (results.size() > 0) {
		testedLines= mystoi(results[0]["testedLines"]);
		durationInSeconds= mystod(results[0]["durationInSeconds"]);
		totalTimeInSeconds= mystod(results[0]["totalTimeInSeconds"]);
		meanInSeconds = mystod(results[0]["averageTime"]);
		slowTimeInSeconds= durationInSeconds > 0.0 ? (meanInSeconds + durationInSeconds) / 2.0 : 0.0;
	} else {
		testedLines= 0;
		durationInSeconds= 0;
		totalTimeInSeconds= 0;
		slowTimeInSeconds= 0;
		meanInSeconds = 0;
	}

	db.exec("SELECT "
				"run_time"
			" FROM run WHERE "
				"name LIKE '" + name + "' "
				"AND options LIKE '"+options+"' "
				"AND header_hash LIKE '"+headerHash+"' "
				"AND test_hash LIKE '"+testHash+"';", &results);

	if (results.size() >= 2) {
		for (auto row = results.begin(); row != results.end(); ++row) {
			times.push_back(mystod((*row)["run_time"]));
		}
		timeStddev = math::stddev(times);
	} else if (results.size() >= 1) {
		timeStddev = mystod(results[0]["run_time"]);
	} else {
		timeStddev = -1.0;
	}
}

void runTest(const String &name, const std::string::size_type maxNameSize, const String &compiler, const io::Path &openssl, Sqlite3::DB &db) {
	String		results;
	String		command;
	String		executableName;
	String		logName;
	String		runLogName;
	String		gcovLogName;
	double		compilePerfTime, compileCoverageTime, runPerfTime, runCoverageTime, totalTime, slowTime;
	uint32_t	coverage;
	uint32_t	uncovered;
	uint32_t	percent_coverage;
	uint32_t	warnings, errors, failures;
	bool		displayNewLine= false;
	String		otherFlags = "";
	String		headerHash;
	String		testHash;
	String		executablePath;
	const String		testSourcePath = "tests/"+name+"_test.cpp";
	const String		headerPath = name+".h";
	const String		options= openssl.isEmpty() ? "" : "openssl";
	uint32_t testedLines;
	double durationInSeconds;
	double meanInSeconds;
	double totalTimeInSeconds;
	double timeStddev;

	hashFile(testSourcePath, testHash);
	hashFile(headerPath, headerHash);
	getTestStats(name, options, headerHash, testHash, testedLines, durationInSeconds, meanInSeconds, timeStddev, totalTimeInSeconds, slowTime, db);
	if (!openssl.isEmpty()) {
		otherFlags = String(" -DOpenSSLAvailable=1 -I") + String(openssl) + String("/include -L") + String(openssl) + String("/lib") + " " + String(gOpensllFlags);
		if (!io::Path(String(openssl) + "/include").isDirectory()) {
			printf(ErrorTextFormatStart "ERROR: openssl directory does have include directory: %s" ClearTextFormat"\n", String(openssl).c_str());
		}
	}
	if(gCompilerLocations[compiler].size() == 0) {
		exec::execute("which "+compiler, results);
		if(stripEOL(results).size() == 0) {
			results= "-";
			printf(WarningTextFormatStart "WARNING: Unable to find compiler %s" ClearTextFormat"\n", compiler.c_str());
		}
		gCompilerLocations[compiler]= results;
		//printf("COMPILER='%s'\n", results.c_str());
	}
	command= gCompilerLocations[compiler];
	if(command != "-") {
		if (totalTimeInSeconds > 0.0) {
			printf((String("%-") + std::to_string(maxNameSize) + String("s %9s about %7.3fs")).c_str(), name.c_str(), compiler.c_str(), totalTimeInSeconds);
		} else {
			printf((String("%-") + std::to_string(maxNameSize) + String("s %9s about   ?????s")).c_str(), name.c_str(), compiler.c_str());
		}
		fflush(stdout);
		executableName= name + '_' + compiler + "_performance";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		executablePath= "bin/tests/"+executableName;
		command+= " -o "+executablePath+" "+testSourcePath+" "
					+(gDebugging ? " -g " : "")+gCompilerFlags+otherFlags+" &> bin/logs/"+logName;
		if (gVerbose) {
			printf("EXECUTING: %s\n", command.c_str());
		}
		compilePerfTime= runNoResultsExpected(command, "compile performance");

		command= executablePath+" &> bin/logs/"+runLogName;
		if (gVerbose) {
			printf("EXECUTING: %s\n", command.c_str());
		}
		runPerfTime= runNoResultsExpected(command, "run performance");

		failures= runIntegerExpected("cat bin/logs/"+runLogName+" | grep FAIL | sort | uniq | wc -l");
		errors= runIntegerExpected("cat bin/logs/"+logName+" | grep error: | sort | uniq | wc -l");
		errors+= runIntegerExpected("cat bin/logs/"+logName+" | grep ld: | sort | uniq | wc -l");
		warnings= runIntegerExpected("cat bin/logs/"+logName+" | grep warning: | sort | uniq | wc -l");

		executableName= name + '_' + compiler + "_trace";
		logName= executableName + "_compile.log";
		runLogName= executableName + "_run.log";
		gcovLogName= executableName + "_gcov.log";
		executablePath= "bin/tests/"+executableName;
		command= gCompilerLocations[compiler]
					+ " -o "+executablePath+" tests/"+name+"_test.cpp "
					+(gDebugging ? " -g " : "")+gCompilerFlags+otherFlags+" -D__Tracer_h__ -fprofile-arcs -ftest-coverage &> bin/logs/"+logName;
		if (gVerbose) {
			printf("EXECUTING: %s\n", command.c_str());
		}
		compileCoverageTime= runNoResultsExpected(command, "compile trace");
		command= executablePath+" &> bin/logs/"+runLogName;
		if (gVerbose) {
			printf("EXECUTING: %s\n", command.c_str());
		}
		runCoverageTime= runNoResultsExpected(command, "run trace");

		exec::execute("mkdir -p bin/coverage/"+executableName+" 2>&1", results);
		if(results != "") {
			printf(WarningTextFormatStart "WARNING: mkdir '%s'" ClearTextFormat"\n", results.c_str());
		}
		exec::execute("gcov "+name+"_test.cpp"+" &> bin/logs/"+gcovLogName, results);
		if(results != "") {
			printf(WarningTextFormatStart "WARNING: gcov '%s'" ClearTextFormat"\n", results.c_str());
		}
		exec::execute("mv *.gcov *.gcno *.gcda bin/coverage/"+executableName+"/ 2>&1", results);
		if(results != "") {
			printf(WarningTextFormatStart "WARNING: mv '%s'" ClearTextFormat"\n", results.c_str());
		}

		coverage= runIntegerExpected("cat bin/coverage/"+executableName+"/"+name+".h.gcov 2> /dev/null | grep -E '[0-9]+:\\s+[0-9]+:' | wc -l");
		uncovered= runIntegerExpected("cat bin/coverage/"+executableName+"/"+name+".h.gcov 2> /dev/null | grep -E '#+:\\s+[0-9]+:' | wc -l");
		percent_coverage= (coverage + uncovered) > 0 ? 100 * coverage / (coverage + uncovered) : 0;
		printf("\t%3d%% coverage\n", percent_coverage);

		if(runPerfTime < gTestMinimumTimeInSeconds * (1.0 + gTestTimeAllowancePercent/100.0) ) {
			printf("\t"  WarningTextFormatStart "Test is too short (%0.5fs), run it %0.1f times" ClearTextFormat"\n", runPerfTime, runPerfTime > 0.0 ? 1.0 / runPerfTime : 10.0);
		}
		if(failures > 0) {
			printf("\t%d Test Failures\n", failures);
			exec::execute("cat bin/logs/"+runLogName+" | grep FAIL", results);
			printf("%s\n", results.c_str());
		}
		if(errors > 0) {
			printf("\t"  ErrorTextFormatStart "%d Compile Errors" ClearTextFormat"\n", errors);
			exec::execute("cat bin/logs/"+logName+" | grep error:", results);
			printf("%s\n", results.c_str());
			exec::execute("cat bin/logs/"+logName+" | grep ld:", results);
			printf("%s\n", results.c_str());
		}
		if(warnings > 0) {
			printf("\t" WarningTextFormatStart "%d Compile Warnings" ClearTextFormat"\n", warnings);
			exec::execute("cat bin/logs/"+logName+" | grep warning:", results);
			printf("%s\n", results.c_str());
		}
		if ((gVerbose && (uncovered > 0)) || (percent_coverage < gMinimumPercentCodeCoverage)) {
			printf("\t" WarningTextFormatStart "warning: %d lines untested (%d tested) %d%%" ClearTextFormat"\n", uncovered, coverage, percent_coverage);
			exec::execute("cat bin/coverage/"+executableName+"/"+name+".h.gcov 2> /dev/null | grep -E '#+:\\s+[0-9]+:'", results);
			printf("%s\n", results.c_str());
		}
		if( (coverage != testedLines) ) {
			printf("\t" WarningTextFormatStart "Tested Lines: %d Expected %d" ClearTextFormat"\n", coverage, testedLines);
			displayNewLine= true;
		}

		if( runPerfTime > durationInSeconds ) {
			printf("\t" ErrorTextFormatStart "Test took %0.3fs, expected less than slowest of %0.3fs" ClearTextFormat"\n", runPerfTime+0.000999, durationInSeconds);
			displayNewLine= true;
		} else if( runPerfTime > meanInSeconds + 2 * timeStddev ) {
			printf("\t" ErrorTextFormatStart "Test took %0.3fs, too slow (%0.3fs average, %0.1fx stddev)" ClearTextFormat"\n",
					runPerfTime+0.000999, meanInSeconds+0.000999, (runPerfTime - meanInSeconds) / timeStddev);
			displayNewLine= true;
		} else if( runPerfTime > meanInSeconds + timeStddev ) {
			printf("\t" WarningTextFormatStart "Test took %0.3fs, too slow (%0.3fs average, %0.1fx stddev)" ClearTextFormat"\n",
					runPerfTime+0.000999, meanInSeconds+0.000999, (runPerfTime - meanInSeconds) / timeStddev);
			displayNewLine= true;
		} else if( runPerfTime < meanInSeconds - timeStddev ) {
			printf("\t" GoodTextFormatStart "Test took %0.3fs, faster than average (%0.3fs average, %0.1fx stddev)" ClearTextFormat"\n", runPerfTime+0.000999, meanInSeconds+0.000999, (meanInSeconds - runPerfTime) / timeStddev);
			displayNewLine= true;
		} else if( runPerfTime < meanInSeconds - 2 * timeStddev ) {
			printf("\t" GreatTextFormatStart "Test took %0.3fs, faster than average (%0.3fs average, %0.1fx stddev)" ClearTextFormat"\n", runPerfTime+0.000999, meanInSeconds+0.000999, (meanInSeconds - runPerfTime) / timeStddev);
			displayNewLine= true;
		}

		totalTime= compilePerfTime + compileCoverageTime + runPerfTime + runCoverageTime;
		if( (totalTime > totalTimeInSeconds) ) {
			printf("\t" ErrorTextFormatStart "Build/Test took %0.3fs, expected %0.3fs" ClearTextFormat"\n", totalTime+0.000999, totalTimeInSeconds);
			displayNewLine= true;
		}
		if(gVerbose) {
			printf("Coverage %d (expected %d) Compile: %0.3fs Run: %0.3fs (expected %0.3fs) Trace Compile: %0.3fs Trace Run: %0.3fs\n",
				coverage, testedLines,
				compilePerfTime, runPerfTime, durationInSeconds, compileCoverageTime, runCoverageTime
			);
		}
		updateRunStats(db, name, headerHash, testHash, compiler, coverage, uncovered, compileCoverageTime, runCoverageTime, compilePerfTime, runPerfTime, options, sourceIdentifier(executablePath+".d"));
	}
}

void runTest(const String &name, const StringList &compilers, const std::string::size_type maxNameSize, const io::Path &openssl, Sqlite3::DB &db) {
	for(auto compiler= compilers.begin(); compiler != compilers.end(); ++compiler) {
		runTest(name, maxNameSize, *compiler, openssl, db);
	}
}

void findFileCoverage(const String &file, const String &options, uint32_t &covered, uint32_t &uncovered, StringList &uncoveredLines, const std::string &testNames, Sqlite3::DB &db) {
	String			results;
	StringList		lines;
	StringList		parts;
	std::map<uint32_t,bool>	coveredLines;

	covered = 0;
	uncovered = 0;
	uncoveredLines.clear();
	exec::execute("cat bin/coverage/*/"+file+".gcov 2> /dev/null | grep -v -E -e '-:\\s+[0-9]+:' | cut -d: -f1-", results);
	split(results, '\n', lines);

	if (strip(results).length() > 0) {
		for (auto line = lines.begin(); line != lines.end(); ++line) {
			split(*line, ':', parts);

			if (parts.size() < 2) {
				continue;
			}

			const bool lineRun = strip(parts[0]).substr(0,1) != "#";
			int lineNumber = std::stoi(strip(parts[1]));

			if (!coveredLines[lineNumber] && lineRun) {
				coveredLines[lineNumber] = true;
			}
		}
		for (auto i = coveredLines.begin(); i != coveredLines.end(); ++i) {
			if (i->second) {
				covered += 1;
			} else {
				uncovered += 1;
			}
		}
		for (auto line = lines.begin(); line != lines.end(); ++line) {
			split(*line, ':', parts);

			if (parts.size() < 2) {
				continue;
			}

			int lineNumber = std::stoi(strip(parts[1]));

			if (!coveredLines[lineNumber]) {
				uncoveredLines.push_back(*line);
				coveredLines[lineNumber] = true;
			}
		}
		updateHeaderStats(db, file, options, covered, uncovered, testNames);
	}
}

/**
	@todo Evaluate performance of compile and run of various compilers
*/
int main(int argc, const char * const argv[]) {
	StringList				testsToRun;
	StringList				compilersToRun;
	io::Path				openssl;
	String					testNames;
	String					testNamePrefix;
	const String			testSuffix= "_test.cpp";
	bool					testsPassed= false;
	const String			compilers = String(",") + gCompilerList + String(",");

	//printf(ErrorTextFormatStart "Error" ClearTextFormat"\n");
	//printf(WarningTextFormatStart "Warning" ClearTextFormat"\n");
	//printf(BoldTextFormatStart "Bold" ClearTextFormat"\n");
	//printf(GoodTextFormatStart "Good" ClearTextFormat"\n");
	//printf(GreatTextFormatStart "Great" ClearTextFormat"\n");

	try {
		io::Path("tests").list(io::Path::NameOnly, testsToRun);
	} catch(const posix::err::ENOENT_Errno &) {
		printf(ErrorTextFormatStart "ERROR: Unable to find tests" ClearTextFormat"\n");
	}
	for (auto i= testsToRun.begin(); i != testsToRun.end();) {
		if ( (i->length() <= testSuffix.length()) || (i->find(testSuffix) != i->length() - testSuffix.length()) ) {
			i= testsToRun.erase(i);
		} else {
			i->erase(i->length() - testSuffix.length());
			++i;
		}
	}
	std::sort(testsToRun.begin(), testsToRun.end());
	for(int arg= 1; arg < argc; ++arg) {
		if(String("debug") == argv[arg]) {
			gDebugging= true;
		} else if(String("list") == argv[arg]) {
			for(auto test= testsToRun.begin(); test != testsToRun.end(); ++test) {
				printf("%s\n", test->c_str());
			}
			testsToRun.clear();
		} else if(String("verbose") == argv[arg]) {
			gVerbose= true;
		} else if(String(argv[arg]).find("openssl=") == 0) {
			openssl = io::Path(String(argv[arg]).substr(8));
			if (!openssl.isDirectory()) {
				printf(WarningTextFormatStart "WARNING: %s is not a directory, disabling openssl" ClearTextFormat"\n", String(openssl).c_str());
				openssl = io::Path();
			} else {
				printf(BoldTextFormatStart "Enabling openssl with headers at: %s" ClearTextFormat"\n", String(openssl).c_str());
			}
		} else if(compilers.find(String(",") + String(argv[arg]) + String(",")) != String::npos) {
			compilersToRun.push_back(argv[arg]);
		} else {
			const bool	found= (io::Path("tests") + (String(argv[arg])+"_test.cpp")).isFile();

			if (found) {
				if (!testsPassed) {
					testsPassed= true;
					testsToRun.clear();
				}
				testsToRun.push_back(argv[arg]);
			} else {
				printf("Compiler/Test not found: %s\n", argv[arg]);
			}
		}
	}
	try {
		String		results;
		StringList	headers;
		String		parentDirectory= io::Path(argv[0]).canonical().parent().parent().name();
		Sqlite3::DB	db(env::get("HOME") + "/Library/Caches/" + parentDirectory + "_tests.sqlite3");

		db.exec("CREATE TABLE IF NOT EXISTS `run` ("
					"`name` VARCHAR(256), "
					"`header_hash` VARCHAR(32), "
					"`test_hash` VARCHAR(32), "
					"`source_identifier` TEXT, "
					"`compiler` VARCHAR(10), "
					"`lines_run` INT, "
					"`code_lines` INT, "
					"`trace_build_time` REAL, "
					"`trace_run_time` REAL, "
					"`build_time` REAL, "
					"`run_time` REAL, "
					"`options` TEXT, "
					"`timestamp` VARCHAR(20));"
		);
		db.exec("CREATE TABLE IF NOT EXISTS `header` ("
					"`name` VARCHAR(256), "
					"`tests` TEXT, "
					"`options` TEXT, "
					"`hash` VARCHAR(32), "
					"`lines_run` INT, "
					"`code_lines` INT, "
					"`timestamp` VARCHAR(20));"
		);
		exec::execute("mkdir -p bin/tests bin/logs", results);
		if(results != "") {
			printf(WarningTextFormatStart "WARNING: mkdir '%s'" ClearTextFormat"\n", results.c_str());
		}
		exec::execute("rm -Rf *.gcov *.gcno *.gcda bin/coverage/* bin/logs/* bin/tests/*", results);
		if(results != "") {
			printf(WarningTextFormatStart "WARNING: rm '%s'" ClearTextFormat"\n", results.c_str());
		}
		std::string::size_type maxNameSize = 0;
		for(auto test= testsToRun.begin(); test != testsToRun.end(); ++test) {
			if (test->size() > maxNameSize) {
				maxNameSize = test->size();
			}
		}
		if (compilersToRun.size() == 0) {
			split(gCompilerList, ',', compilersToRun);
		}
		for(auto test= testsToRun.begin(); test != testsToRun.end(); ++test) {
			runTest(*test, compilersToRun, maxNameSize, openssl, db);
			testNames= testNames + testNamePrefix + *test;
			if (testNamePrefix.length() == 0) {
				testNamePrefix= ",";
			}
		}
		if(testsToRun.size() > 0) {
			printf("Examining overall coverage ...\n");
			exec::execute("ls *.h", results);
			split(results, '\n', headers);
			for(auto header= headers.begin(); header != headers.end(); ++header) {
				uint32_t	coverage;
				uint32_t	uncovered;
				StringList	uncoveredLines;
				int			expectedLinesRun= 0, expectedLinesNotRun= 0, bestCoverage;
				std::string	headerHash;

				hashFile(*header, headerHash);
				getHeaderStats(db, *header, headerHash, openssl.isEmpty() ? "" : "openssl", expectedLinesRun, expectedLinesNotRun, testNames, bestCoverage);
				findFileCoverage(*header, openssl.isEmpty() ? "" : "openssl", coverage, uncovered, uncoveredLines, testNames, db);

				const bool hasExpectations= (expectedLinesRun > 0) || (expectedLinesNotRun > 0);
				const bool	hasCoverage= (coverage > 0) || (uncovered > 0);
				const bool	unexpectedCoverage= (coverage != uint32_t(expectedLinesRun));
				const bool	unexpectedLines= (uncovered != uint32_t(expectedLinesNotRun));
				const int	coverageRate= hasCoverage ? 100 * coverage / (coverage + uncovered) : 0;

				if ( (hasExpectations || hasCoverage) && (unexpectedCoverage || unexpectedLines) ) {
					printf(WarningTextFormatStart "%s coverage changed %d/%d -> %d/%d (%d%% -> %d%%)" ClearTextFormat"\n",
						header->c_str(),
						expectedLinesRun,
							expectedLinesRun + expectedLinesNotRun,
						coverage,
							coverage + uncovered,
						hasExpectations ? 100 * expectedLinesRun / (expectedLinesRun + expectedLinesNotRun) : 0,
						coverageRate
					);
					for (auto i= uncoveredLines.begin(); i != uncoveredLines.end(); ++i) {
						printf("%s\n", i->c_str());
					}
				} if ( coverageRate < bestCoverage ) {
					printf(ErrorTextFormatStart "%s coverage is lower than best %d/%d (%d%%) < %d%%" ClearTextFormat"\n",
						header->c_str(),
						coverage,
						coverage + uncovered,
						coverageRate,
						bestCoverage
					);
					for (auto i= uncoveredLines.begin(); i != uncoveredLines.end(); ++i) {
						printf("%s\n", i->c_str());
					}
				}
			}
		}
		// dumpCompilerStats(db); TODO: Need to determine what is wrong here
	} catch(const std::exception &exception) {
		printf("EXCEPTION: %s\n", exception.what());
	}
	return 0;
}
