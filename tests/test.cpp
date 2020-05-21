#include "os/DateTime.h"
#include "os/Environment.h"
#include "os/Exception.h"
#include "os/Execute.h"
#include "os/File.h"
#include "os/Hash.h"
#include "os/Path.h"
#include "os/Sqlite3Plus.h"
#include "os/Statistics.h"
#include <algorithm> // std::find
#include <ctype.h>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

// select name, compiler, options, count(*) as count, avg(run_time) as
// average_run_time, min(run_time) as min_run_time, max(run_time) as
// max_run_time, avg(100*lines_run/code_lines) as average_coverage,
// min(100*lines_run/code_lines) as min_coverage, max(100*lines_run/code_lines)
// as max_coverage from run group by name, test_hash, header_hash, compiler,
// options; select name, compiler, options, count(*) as count, avg(run_time) as
// average_run_time, min(run_time) as min_run_time, max(run_time) as
// max_run_time, avg(100*lines_run/code_lines) as average_coverage,
// min(100*lines_run/code_lines) as min_coverage, max(100*lines_run/code_lines)
// as max_coverage from run group by name, source_identifier, compiler, options;

struct Times {
  double perfCompile, perfRun, traceCompile, traceRun;
  uint32_t testedLines, warnings;
};
typedef std::string String;
typedef std::vector<String> StringList;
typedef std::map<String, String> Dictionary;

const double gTestTimeAllowancePercent = 5;
const double gTestMinimumTimeInSeconds = 1;
const char *const gOpensllFlags = "-lcrypto";
const char *const gCompilerFlags =
    "-I.. -MMD -std=c++11"
    " -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings"
#if defined(__linux__)
    " -lcrypto -DOpenSSLAvailable=1"
#endif
#if defined(__APPLE__)
    " -framework CoreFoundation"
#endif
    " -lz -lsqlite3";
const char *const gDebugFlags =
#if defined(__APPLE__)
    " -fsanitize=address -fsanitize-address-use-after-scope"
    " -fsanitize=undefined"
#endif
    " -fno-optimize-sibling-calls -O0 -g ";

const uint32_t gMinimumPercentCodeCoverage = 70;
#if defined(__APPLE__)
const String gCompilerList = "clang++"; //,g++,llvm-g++";
#elif defined(__linux__)
// may do clang++ later, but need to figure out llvm-cov (and maybe install
// llvm)
const String gCompilerList = "g++"; //,clang++,llvm-g++";
#endif

Dictionary gCompilerLocations;
bool gDebugging = false;
bool gVerbose = false;

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
  while ((s.size() > 0) &&
         ((s[s.size() - 1] == '\n') || (s[s.size() - 1] == '\r'))) {
    s.erase(s.size() - 1);
  }
  return s;
}

String &strip(String &s) {
  while ((s.size() > 0) && isspace(s[0])) {
    s.erase(0, 1);
  }
  while ((s.size() > 0) && isspace(s[s.size() - 1])) {
    s.erase(s.size() - 1);
  }
  return s;
}

StringList &split(const String &string, const char character,
                  StringList &parts) {
  String::size_type characterPos, start = 0;

  parts.clear();
  do {
    characterPos = string.find(character, start);
    if (String::npos == characterPos) {
      characterPos = string.size();
    }
    parts.push_back(string.substr(start, characterPos - start));
    start = characterPos + 1;
  } while (start < string.size());
  return parts;
}

int runIntegerExpected(const String &command) {
  String results;

  return std::stoi(strip(exec::execute(command, results)));
}

double runNoResultsExpected(const String &command, const char *const action) {
  String results;
  double duration;
  dt::DateTime start;

  exec::execute(command, results);
  duration = dt::DateTime() - start;
  if (stripEOL(results).size() != 0) {
    printf("unexpected %s result '%s'\n", action, results.c_str());
  }

  return duration;
}

String fileContents(const String &path) {
  try {
    return io::Path(path).contents();
  } catch (const posix::err::Errno &) {
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
  } catch (const std::exception &) {
  }
  return 0;
}

double mystod(const std::string &number) {
  try {
    return std::stod(number);
  } catch (const std::exception &) {
  }
  return 0.0;
}

void getHeaderStats(Sqlite3::DB &db, const String &name,
                    const String &headerHash, const String &options,
                    int &linesRun, int &linesNotRun,
                    const std::string &testNames, int &bestPercent) {
  Sqlite3::DB::Results results;

  db.exec("SELECT name,lines_run,code_lines,timestamp FROM header WHERE name "
          "LIKE '" +
              name + "' AND tests LIKE '" + testNames + "' AND options LIKE '" +
              options + "' ORDER BY timestamp DESC;",
          &results);

  if (results.size() > 0) {
    linesRun = results[0]("lines_run", Sqlite3::IntegerType).integer();
    linesNotRun =
        results[0]("code_lines", Sqlite3::IntegerType).integer() - linesRun;
  } else {
    linesRun = 0;
    linesNotRun = 0;
  }
  db.exec("SELECT MAX(100 * lines_run / code_lines) AS coverage FROM header "
          "WHERE name LIKE '" +
              name + "' AND tests like '" + testNames + "' AND hash LIKE '" +
              headerHash + "';",
          &results);
  if (results.size() > 0) {
    bestPercent = results[0]("coverage", Sqlite3::IntegerType).integer();
  } else {
    bestPercent = 0;
  }
}

void updateHeaderStats(Sqlite3::DB &db, const String &name,
                       const String &options, int linesRun, int linesNotRun,
                       const std::string &testNames) {
  Sqlite3::Row row;
  String buffer;

  row["name"] = name;
  row["options"] = options;
  row["hash"] = hashFile(name, buffer);
  row["lines_run"] = std::to_string(linesRun);
  row["code_lines"] = std::to_string(linesRun + linesNotRun);
  row["tests"] = testNames;
  row["timestamp"] = dt::DateTime().format("%Y/%m/%d %H:%M:%S", buffer);
  db.addRow("header", row);
}

void updateRunStats(Sqlite3::DB &db, const String &name,
                    const String &headerHash, const String &testHash,
                    const String &compiler, int linesRun, int linesNotRun,
                    double traceBuildTime, double traceRunTime,
                    double buildTime, double runTime, const String &options,
                    const String &sourceIdentifier) {
  Sqlite3::Row row;
  String buffer;

  row["name"] = name;
  row["header_hash"] = headerHash;
  row["test_hash"] = testHash;
  row["compiler"] = compiler;
  row["lines_run"] = std::to_string(linesRun);
  row["code_lines"] = std::to_string(linesRun + linesNotRun);
  row["trace_build_time"] = std::to_string(traceBuildTime);
  row["trace_run_time"] = std::to_string(traceRunTime);
  row["build_time"] = std::to_string(buildTime);
  row["run_time"] = std::to_string(runTime);
  row["options"] = options;
  row["source_identifier"] = sourceIdentifier;
  row["timestamp"] = dt::DateTime().format("%Y/%m/%d %H:%M:%S", buffer);
  db.addRow("run", row);
}

String::size_type skip(const String &text, String::size_type &index,
                       bool whitespace) {
  while ((index < text.length()) &&
         ((::isspace(text[index]) != 0) == whitespace)) {
    ++index;
  }
  return index;
}

void removeLineEndingEscapes(String &text) {
  String::size_type backslash = 0;

  while (backslash != String::npos) {
    if (String::npos != (backslash = text.find("\\\n"))) {
      text.replace(backslash, 2, 1, ' ');
    } else if (String::npos != (backslash = text.find("\\\r\n"))) {
      text.replace(backslash, 3, 1, ' ');
    } else if (String::npos != (backslash = text.find("\\\r"))) {
      text.replace(backslash, 2, 1, ' ');
    }
  }
}

String sourceIdentifier(const String &dependenciesPath) {
  String contents = fileContents(dependenciesPath);
  String::size_type startPos = contents.find(':');
  String identifier("");
  String hashBuffer;
  String prefix("");

  removeLineEndingEscapes(contents);
  if (String::npos != startPos) {
    String::size_type endPos;
    String path;

    ++startPos;
    while (startPos < contents.length()) {
      skip(contents, startPos, true);
      endPos = startPos;
      skip(contents, endPos, false);
      strip(path.assign(contents, startPos, endPos - startPos));
      if (path.length() > 0) {
        identifier += prefix + path + ":" + hashFile(path, hashBuffer);
        prefix = ",";
      }
      startPos = endPos;
    }
  }
  return identifier;
}

typedef std::pair<math::List, math::List> BuildAndRunTimesList;
typedef std::map<String, BuildAndRunTimesList> CompilerBuildAndRunTimesList;
void updateCompilerTimesIfNeeded(
    CompilerBuildAndRunTimesList &compilerTimes,
    CompilerBuildAndRunTimesList &compilerDifferences, const String &compiler,
    const String &source, String &currentCompiler, String &currentSource,
    double &buildSum, double &runSum, double &count) {
  const bool sourceChanged = currentSource != source;
  const bool compilerChanged = currentCompiler != compiler;

  if (sourceChanged || compilerChanged) {
    if (count > 0) {
      compilerTimes[currentCompiler].first.push_back(buildSum / count);
      compilerTimes[currentCompiler].second.push_back(runSum / count);
    }

    if (sourceChanged && (compilerTimes.size() > 0)) {
      std::map<String, double> compilerAverages;
      double minBuild = std::numeric_limits<double>::max();
      double minRun = std::numeric_limits<double>::max();

      for (auto compilerAndTimes : compilerTimes) {
        double buildTime = math::mean(compilerAndTimes.second.first);
        double runTime = math::mean(compilerAndTimes.second.second);

        if (buildTime < minBuild) {
          minBuild = buildTime;
        }
        if (runTime < minRun) {
          minRun = runTime;
        }
      }
      for (auto compilerAndTimes : compilerTimes) {
        double buildTime = math::mean(compilerAndTimes.second.first);
        double runTime = math::mean(compilerAndTimes.second.second);

        printf("%s build %7.10f%% %7.10fs min %7.10fs\n",
               compilerAndTimes.first.c_str(), 100 * buildTime / minBuild,
               buildTime, minBuild);
        printf("%s run   %7.10f%% %7.10fs min %7.10fs\n",
               compilerAndTimes.first.c_str(), 100 * runTime / minRun, runTime,
               minRun);
        compilerDifferences[compilerAndTimes.first].first.push_back(buildTime /
                                                                    minBuild);
        compilerDifferences[compilerAndTimes.first].second.push_back(runTime /
                                                                     minRun);
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

void getTestStats(const String &name, const String &options,
                  const String &headerHash, const String &testHash,
                  uint32_t &testedLines, double &durationInSeconds,
                  double &meanInSeconds, double &timeStddev,
                  double &totalTimeInSeconds, double &slowTimeInSeconds,
                  Sqlite3::DB &db) {
  Sqlite3::DB::Results results;
  math::List times;

  db.exec("SELECT "
          "MAX(lines_run) AS testedLines, "
          "MAX(run_time) AS durationInSeconds, "
          "MAX(trace_build_time + trace_run_time + build_time + run_time) AS "
          "totalTimeInSeconds, "
          "AVG(run_time) AS averageTime "
          "FROM run WHERE "
          "name LIKE '" +
              name +
              "' "
              "AND options LIKE '" +
              options +
              "' "
              "AND header_hash LIKE '" +
              headerHash +
              "' "
              "AND test_hash LIKE '" +
              testHash + "';",
          &results);

  if (results.size() > 0) {
    testedLines = results[0]("testedLines", Sqlite3::IntegerType).integer();
    durationInSeconds =
        results[0]("durationInSeconds", Sqlite3::RealType).real();
    totalTimeInSeconds =
        results[0]("totalTimeInSeconds", Sqlite3::RealType).real();
    meanInSeconds = results[0]("averageTime", Sqlite3::RealType).real();
    slowTimeInSeconds = durationInSeconds > 0.0
                            ? (meanInSeconds + durationInSeconds) / 2.0
                            : 0.0;
  } else {
    testedLines = 0;
    durationInSeconds = 0;
    totalTimeInSeconds = 0;
    slowTimeInSeconds = 0;
    meanInSeconds = 0;
  }

  db.exec("SELECT "
          "run_time"
          " FROM run WHERE "
          "name LIKE '" +
              name +
              "' "
              "AND options LIKE '" +
              options +
              "' "
              "AND header_hash LIKE '" +
              headerHash +
              "' "
              "AND test_hash LIKE '" +
              testHash + "';",
          &results);

  if (results.size() >= 2) {
    for (auto row : results) {
      times.push_back(row("run_time", Sqlite3::RealType).real());
    }
    timeStddev = math::stddev(times);
  } else if (results.size() >= 1) {
    timeStddev = results[0]("run_time", Sqlite3::RealType).real();
  } else {
    timeStddev = -1.0;
  }
}

void runTest(const String &name, const std::string::size_type maxNameSize,
             const String &compiler, const io::Path &openssl, Sqlite3::DB &db) {
  String results;
  String command;
  String executableName;
  String logName;
  String runLogName;
  String gcovLogName;
  double compilePerfTime, compileCoverageTime, runPerfTime, runCoverageTime,
      totalTime, slowTime;
  uint32_t coverage;
  uint32_t uncovered;
  uint32_t percent_coverage;
  uint32_t warnings, errors, failures;
  bool displayNewLine = false;
  String otherFlags = "";
  String headerHash;
  String testHash;
  String executablePath;
  const String testSourcePath = "tests/" + name + "_test.cpp";
  const String headerPath = name + ".h";
  const String options = openssl.isEmpty() ? "" : "openssl";
  uint32_t testedLines;
  double durationInSeconds;
  double meanInSeconds;
  double totalTimeInSeconds;
  double timeStddev;

  hashFile(testSourcePath, testHash);
  hashFile(headerPath, headerHash);
  getTestStats(name, options, headerHash, testHash, testedLines,
               durationInSeconds, meanInSeconds, timeStddev, totalTimeInSeconds,
               slowTime, db);
  if (!openssl.isEmpty()) {
    otherFlags = String(" -DOpenSSLAvailable=1 -I") + String(openssl) +
                 String("/include -L") + String(openssl) + String("/lib") +
                 " " + String(gOpensllFlags);
    if (!io::Path(String(openssl) + "/include").isDirectory()) {
      printf(ErrorTextFormatStart "ERROR: openssl directory does have include "
                                  "directory: %s" ClearTextFormat "\n",
             String(openssl).c_str());
    }
  }
  if (gCompilerLocations[compiler].size() == 0) {
    exec::execute("which " + compiler, results);
    if (stripEOL(results).size() == 0) {
      results = "-";
      printf(WarningTextFormatStart
             "WARNING: Unable to find compiler %s" ClearTextFormat "\n",
             compiler.c_str());
    }
    gCompilerLocations[compiler] = results;
    // printf("COMPILER='%s'\n", results.c_str());
  }
  command = gCompilerLocations[compiler];
  if (command != "-") {
    if (totalTimeInSeconds > 0.0) {
      printf((String("%-") + std::to_string(maxNameSize) +
              String("s %9s about %7.3fs"))
                 .c_str(),
             name.c_str(), compiler.c_str(), totalTimeInSeconds);
    } else {
      printf((String("%-") + std::to_string(maxNameSize) +
              String("s %9s about   ?????s"))
                 .c_str(),
             name.c_str(), compiler.c_str());
    }
    fflush(stdout);
    executableName = name + '_' + compiler + "_performance";
    logName = executableName + "_compile.log";
    runLogName = executableName + "_run.log";
    executablePath = "bin/tests/" + executableName;
    command += " -o " + executablePath + " " + testSourcePath + " " +
               (gDebugging ? gDebugFlags : "") + gCompilerFlags + otherFlags +
               " &> bin/logs/" + logName;
    if (gVerbose) {
      printf("\nEXECUTING: %s\n", command.c_str());
    }
    compilePerfTime = runNoResultsExpected(command, "compile performance");

    command = executablePath + " &> bin/logs/" + runLogName;
    if (gVerbose) {
      printf("EXECUTING: %s\n", command.c_str());
    }
    runPerfTime = runNoResultsExpected(command, "run performance");

    failures = runIntegerExpected("cat bin/logs/" + runLogName +
                                  " | grep FAIL | sort | uniq | wc -l");
    errors = runIntegerExpected("cat bin/logs/" + logName +
                                " | grep error: | sort | uniq | wc -l");
    errors += runIntegerExpected("cat bin/logs/" + logName +
                                 " | grep ld: | sort | uniq | wc -l");
    warnings = runIntegerExpected("cat bin/logs/" + logName +
                                  " | grep warning: | sort | uniq | wc -l");

    executableName = name + '_' + compiler + "_trace";
    logName = executableName + "_compile.log";
    runLogName = executableName + "_run.log";
    gcovLogName = executableName + "_gcov.log";
    executablePath = "bin/tests/" + executableName;
    command =
        gCompilerLocations[compiler] + " -o " + executablePath + " tests/" +
        name + "_test.cpp " + gDebugFlags + gCompilerFlags + otherFlags +
        " -D__Tracer_h__ -fprofile-arcs -ftest-coverage -g -O0 &> bin/logs/" +
        logName;
    if (gVerbose) {
      printf("EXECUTING: %s\n", command.c_str());
    }
    compileCoverageTime = runNoResultsExpected(command, "compile trace");
    command = executablePath + " &> bin/logs/" + runLogName;
    if (gVerbose) {
      printf("EXECUTING: %s\n", command.c_str());
    }
    runCoverageTime = runNoResultsExpected(command, "run trace");

    exec::execute("mkdir -p bin/coverage/" + executableName + " 2>&1", results);
    if (results != "") {
      printf(WarningTextFormatStart "WARNING: mkdir '%s'" ClearTextFormat "\n",
             results.c_str());
    }
    command =
        String("gcov ") + name + "_test.cpp" + " &> bin/logs/" + gcovLogName;
    if (gVerbose) {
      printf("EXECUTING: %s\n", command.c_str());
    }
    exec::execute(command, results);
    if (results != "") {
      printf(WarningTextFormatStart "WARNING: gcov '%s'" ClearTextFormat "\n",
             results.c_str());
    }
    exec::execute("mv *.gcov *.gcno *.gcda bin/coverage/" + executableName +
                      "/ 2>&1",
                  results);
    if (results != "") {
      printf(WarningTextFormatStart "WARNING: mv '%s'" ClearTextFormat "\n",
             results.c_str());
    }

    coverage = runIntegerExpected(
        "cat bin/coverage/" + executableName + "/" + name +
        ".h.gcov 2> /dev/null | grep -E '[0-9]+:\\s+[0-9]+:' | wc -l");
    uncovered = runIntegerExpected(
        "cat bin/coverage/" + executableName + "/" + name +
        ".h.gcov 2> /dev/null | grep -E '#+:\\s+[0-9]+:' | wc -l");
    percent_coverage = (coverage + uncovered) > 0
                           ? 100 * coverage / (coverage + uncovered)
                           : 0;
    printf("\t%3d%% coverage\n", percent_coverage);

    if (runPerfTime <
        gTestMinimumTimeInSeconds * (1.0 + gTestTimeAllowancePercent / 100.0)) {
      printf("\t" WarningTextFormatStart
             "Test is too short (%0.5fs), run it %0.1f times" ClearTextFormat
             "\n",
             runPerfTime, runPerfTime > 0.0 ? 1.0 / runPerfTime : 10.0);
    }
    if (failures > 0) {
      printf("\t%d Test Failures\n", failures);
      exec::execute("cat bin/logs/" + runLogName + " | grep FAIL", results);
      printf("%s\n", results.c_str());
    }
    if (errors > 0) {
      printf("\t" ErrorTextFormatStart "%d Compile Errors" ClearTextFormat "\n",
             errors);
      exec::execute("cat bin/logs/" + logName + " | grep error:", results);
      printf("%s\n", results.c_str());
      exec::execute("cat bin/logs/" + logName + " | grep ld:", results);
      printf("%s\n", results.c_str());
    }
    if (warnings > 0) {
      printf("\t" WarningTextFormatStart "%d Compile Warnings" ClearTextFormat
             "\n",
             warnings);
      exec::execute("cat bin/logs/" + logName + " | grep warning:", results);
      printf("%s\n", results.c_str());
    }
    if ((gVerbose && (uncovered > 0)) ||
        (percent_coverage < gMinimumPercentCodeCoverage)) {
      printf("\t" WarningTextFormatStart
             "warning: %d lines untested (%d tested) %d%%" ClearTextFormat "\n",
             uncovered, coverage, percent_coverage);
      exec::execute("cat bin/coverage/" + executableName + "/" + name +
                        ".h.gcov 2> /dev/null | grep -E '#+:\\s+[0-9]+:'",
                    results);
      printf("%s\n", results.c_str());
    }
    if ((coverage != testedLines)) {
      printf("\t" WarningTextFormatStart
             "Tested Lines: %d Expected %d" ClearTextFormat "\n",
             coverage, testedLines);
      displayNewLine = true;
    }

    if (runPerfTime > durationInSeconds) {
      printf("\t" ErrorTextFormatStart "Test took %0.3fs, expected less than "
             "slowest of %0.3fs" ClearTextFormat "\n",
             runPerfTime + 0.000999, durationInSeconds);
      displayNewLine = true;
    } else if (runPerfTime > meanInSeconds + 2 * timeStddev) {
      printf("\t" ErrorTextFormatStart "Test took %0.3fs, too slow (%0.3fs "
             "average, %0.1fx stddev)" ClearTextFormat "\n",
             runPerfTime + 0.000999, meanInSeconds + 0.000999,
             (runPerfTime - meanInSeconds) / timeStddev);
      displayNewLine = true;
    } else if (runPerfTime > meanInSeconds + timeStddev) {
      printf("\t" WarningTextFormatStart
             "Test took %0.3fs, too slow (%0.3fs average, %0.1fx "
             "stddev)" ClearTextFormat "\n",
             runPerfTime + 0.000999, meanInSeconds + 0.000999,
             (runPerfTime - meanInSeconds) / timeStddev);
      displayNewLine = true;
    } else if (runPerfTime < meanInSeconds - timeStddev) {
      printf("\t" GoodTextFormatStart
             "Test took %0.3fs, faster than average (%0.3fs average, %0.1fx "
             "stddev)" ClearTextFormat "\n",
             runPerfTime + 0.000999, meanInSeconds + 0.000999,
             (meanInSeconds - runPerfTime) / timeStddev);
      displayNewLine = true;
    } else if (runPerfTime < meanInSeconds - 2 * timeStddev) {
      printf("\t" GreatTextFormatStart
             "Test took %0.3fs, faster than average (%0.3fs average, %0.1fx "
             "stddev)" ClearTextFormat "\n",
             runPerfTime + 0.000999, meanInSeconds + 0.000999,
             (meanInSeconds - runPerfTime) / timeStddev);
      displayNewLine = true;
    }

    totalTime =
        compilePerfTime + compileCoverageTime + runPerfTime + runCoverageTime;
    if ((totalTime > totalTimeInSeconds)) {
      printf("\t" ErrorTextFormatStart
             "Build/Test took %0.3fs, expected %0.3fs" ClearTextFormat "\n",
             totalTime + 0.000999, totalTimeInSeconds);
      displayNewLine = true;
    }
    if (gVerbose) {
      printf("Coverage %d (expected %d) Compile: %0.3fs Run: %0.3fs (expected "
             "%0.3fs) Trace Compile: %0.3fs Trace Run: %0.3fs\n",
             coverage, testedLines, compilePerfTime, runPerfTime,
             durationInSeconds, compileCoverageTime, runCoverageTime);
    }
    updateRunStats(db, name, headerHash, testHash, compiler, coverage,
                   uncovered, compileCoverageTime, runCoverageTime,
                   compilePerfTime, runPerfTime, options,
                   sourceIdentifier(executablePath + ".d"));
  }
}

void runTest(const String &name, const StringList &compilers,
             const std::string::size_type maxNameSize, const io::Path &openssl,
             Sqlite3::DB &db) {
  for (auto compiler : compilers) {
    runTest(name, maxNameSize, compiler, openssl, db);
  }
}

// TODO: Filter out lines where the only source is a close bracket
void findFileCoverage(const String &file, const String &options,
                      uint32_t &covered, uint32_t &uncovered,
                      StringList &uncoveredLines, const std::string &testNames,
                      Sqlite3::DB &db) {
  String results;
  StringList lines;
  StringList parts;
  std::map<uint32_t, bool> coveredLines;

  covered = 0;
  uncovered = 0;
  uncoveredLines.clear();
  exec::execute(
      "cat bin/coverage/*/" + file +
          ".gcov 2> /dev/null | grep -v -E -e '-:\\s+[0-9]+:' | cut -d: -f1-",
      results);
  split(results, '\n', lines);

  if (strip(results).length() > 0) {
    for (auto line : lines) {
      split(line, ':', parts);

      if (parts.size() < 2) {
        continue; // not a validate source line
      }

      if (strip(parts[2]) == "}") {
        continue; // just a close bracket (destructors)
      }

      const bool lineRun = strip(parts[0]).substr(0, 1) != "#";
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
    for (auto line : lines) {
      split(line, ':', parts);

      if (parts.size() < 2) {
        continue;
      }

      if (strip(parts[2]) == "}") {
        continue; // just a close bracket (destructors)
      }

      int lineNumber = std::stoi(strip(parts[1]));

      if (!coveredLines[lineNumber]) {
        uncoveredLines.push_back(line);
        coveredLines[lineNumber] = true;
      }
    }
    updateHeaderStats(db, file, options, covered, uncovered, testNames);
  }
}

void reportRun(const std::string &reason, const std::string test,
               const std::string &startTest, const std::string &startSource,
               const std::string &end, const math::List &testRuns,
               const math::List &sourceRuns) {
  double testMean, testStdDev = 0.0, sourceMean, sourceStdDev = 0.0;
  double sumValue, varianceValue;
  math::List testRunsWithOtherSource = testRuns;
  const void *unused[] = {&unused, &reason};
  math::List filteredSourceRuns = sourceRuns;
  math::List filteredTestRunsWithOtherSource;
  if (!io::Path(test + ".h").isFile()) {
    return;
  }

  if (sourceRuns.size() >= 2) {
    math::statistics(sourceRuns, sourceMean, sumValue, varianceValue,
                     sourceStdDev);
    math::filterInRange(filteredSourceRuns, sourceMean - 2 * sourceStdDev,
                        sourceMean + 2 * sourceStdDev);
  } else {
    sourceMean = math::mean(sourceRuns);
  }

  testRunsWithOtherSource.erase(testRunsWithOtherSource.begin() +
                                    (testRuns.size() - sourceRuns.size()),
                                testRunsWithOtherSource.end());

  if (testRunsWithOtherSource.size() == 0) {
    printf("%s\n"
           "\t test last changed %s source last changed %s as of %s\n"
           "\t average test   run %4ld times run time = %5.1f "
           "seconds (%5.1f - %5.1f)\n",
           test.c_str(),                                        // test name
           startTest.c_str(), startSource.c_str(), end.c_str(), // dates
           sourceRuns.size(), sourceMean,                       // source stats
           math::min(filteredSourceRuns),
           math::max(filteredSourceRuns)); // source range
    return;
  }

  filteredTestRunsWithOtherSource = testRunsWithOtherSource;

  if (testRunsWithOtherSource.size() >= 2) {
    math::statistics(testRunsWithOtherSource, testMean, sumValue, varianceValue,
                     testStdDev);
    math::filterInRange(filteredTestRunsWithOtherSource,
                        testMean - 2 * testStdDev, testMean + 2 * testStdDev);
  } else {
    testMean = math::mean(testRunsWithOtherSource);
  }

  const bool good = sourceMean <= testMean;
  const bool great = good && (sourceMean < testMean - testStdDev);
  const bool poor = sourceMean > testMean;
  const bool bad = poor && (sourceMean > testMean + testStdDev);
  const char *color =
      bad ? ErrorTextFormatStart
          : (poor ? WarningTextFormatStart
                  : (great ? GreatTextFormatStart : GoodTextFormatStart));

  printf("%s\n"
         "\t test last changed %s source last changed %s as of %s\n"
         "\t average test   run %4ld times run time = %5.1f "
         "seconds (%5.1f - %5.1f)\n"
         "\t %saverage source run %4ld times run time = %5.1f "
         "seconds (%5.1f - %5.1f)%s\n",
         test.c_str(),                                        // test name
         startTest.c_str(), startSource.c_str(), end.c_str(), // dates
         testRunsWithOtherSource.size(), testMean,            // test stats
         math::min(filteredTestRunsWithOtherSource),
         math::max(filteredTestRunsWithOtherSource), // test range
         color,                                      /// start color
         sourceRuns.size(), sourceMean,              // source stats
         math::min(filteredSourceRuns),
         math::max(filteredSourceRuns), // source range
         ClearTextFormat);
}

void performanceReport(Sqlite3::DB &db, const StringList &testsToRun,
                       bool fullReport = false) {
  Sqlite3::DB::Results results;
  std::string test, test_hash, source_identifier, startTest, startSource, last;
  std::string reason;
  math::List testRuns;
  math::List sourceRuns;

  db.exec("SELECT test_hash,source_identifier,timestamp,name,run_time FROM run "
          "ORDER BY cast(name as text),cast(timestamp as text);",
          &results);

  for (auto row : results) {
    const std::string testName = row("name", Sqlite3::TextType).text();

    if (!fullReport && (std::find(testsToRun.begin(), testsToRun.end(),
                                  testName) == testsToRun.end())) {
      continue;
    }

    if (testName != test) {
      if (testRuns.size() > 0) {
        reportRun(reason, test, startTest, startSource, last, testRuns,
                  sourceRuns);
      }
      test = testName;
      test_hash = row("test_hash", Sqlite3::TextType).text();
      source_identifier = row("source_identifier", Sqlite3::TextType).text();
      startTest = row("timestamp", Sqlite3::TextType).text();
      startSource = row("timestamp", Sqlite3::TextType).text();
      last = row("timestamp", Sqlite3::TextType).text();
      testRuns.clear();
      testRuns.push_back(row("run_time", Sqlite3::RealType).real());
      sourceRuns.clear();
      sourceRuns.push_back(row("run_time", Sqlite3::RealType).real());
      reason = "Started testing";
    } else if (row("test_hash", Sqlite3::TextType).text() != test_hash) {
      if (fullReport) {
        reportRun(reason, test, startTest, startSource, last, testRuns,
                  sourceRuns);
      }
      // test = testName;
      test_hash = row("test_hash", Sqlite3::TextType).text();
      source_identifier = row("source_identifier", Sqlite3::TextType).text();
      startTest = row("timestamp", Sqlite3::TextType).text();
      startSource = row("timestamp", Sqlite3::TextType).text();
      last = row("timestamp", Sqlite3::TextType).text();
      testRuns.clear();
      testRuns.push_back(row("run_time", Sqlite3::RealType).real());
      sourceRuns.clear();
      sourceRuns.push_back(row("run_time", Sqlite3::RealType).real());
      reason = "Test changed   ";
    } else if (row("source_identifier", Sqlite3::TextType).text() !=
               source_identifier) {
      if (fullReport) {
        reportRun(reason, test, startTest, startSource, last, testRuns,
                  sourceRuns);
      }
      // test = testName;
      // test_hash = row("test_hash", Sqlite3::TextType).text();
      source_identifier = row("source_identifier", Sqlite3::TextType).text();
      // startTest = row("timestamp", Sqlite3::TextType).text();
      startSource = row("timestamp", Sqlite3::TextType).text();
      last = row("timestamp", Sqlite3::TextType).text();
      // testRuns.clear();
      testRuns.push_back(row("run_time", Sqlite3::RealType).real());
      sourceRuns.clear();
      sourceRuns.push_back(row("run_time", Sqlite3::RealType).real());
      reason = "Source changed ";
    } else {
      if (fullReport) {
        reportRun(reason, test, startTest, startSource, last, testRuns,
                  sourceRuns);
      }
      // test = testName;
      // test_hash = row("test_hash", Sqlite3::TextType).text();
      // source_identifier = row("source_identifier", Sqlite3::TextType).text();
      // startTest = row("timestamp", Sqlite3::TextType).text();
      // startSource = row("timestamp", Sqlite3::TextType).text();
      last = row("timestamp", Sqlite3::TextType).text();
      // testRuns.clear();
      testRuns.push_back(row("run_time", Sqlite3::RealType).real());
      // sourceRuns.clear();
      sourceRuns.push_back(row("run_time", Sqlite3::RealType).real());
      // reason = "Test run       ";
    }
  }

  reportRun(reason, test, startTest, startSource, last, testRuns, sourceRuns);
}

double getTime(const math::List &times) {

  if (times.size() == 0) {
    return 0.0;
  }

  if (times.size() == 1) {
    return times[0];
  }

  double mean, sum, variance, stdev;
  math::List timesCopy = times;

  math::statistics(timesCopy, mean, sum, variance, stdev);
  /*
  math::filterInRange(timesCopy, mean - std::min(stdev,0.0), mean +
  std::min(stdev, 0.0));

      if (timesCopy.size() == 0) {
              return 0.0;
      }

      if (timesCopy.size() == 1) {
              return times[0];
      }

  math::statistics(timesCopy, mean, sum, variance, stdev);
  */
  return mean;
}

typedef std::map<std::string, math::List> NamedMathList;
void analyze(const NamedMathList &build, const NamedMathList &run,
             NamedMathList &builds, NamedMathList &runs,
             const StringList &compilers) {
  for (auto compiler : compilers) {
    if ((build.find(compiler) == build.end()) ||
        (run.find(compiler) == run.end())) {
      return;
    }
    if (build.at(compiler).size() < 2) {
      return;
    }
  }

  auto primeCompiler = compilers[0];
  auto otherCompilers = compilers;
  const double buildPrimeTime = getTime(build.at(primeCompiler));
  const double runPrimeTime = getTime(run.at(primeCompiler));

  builds[primeCompiler].push_back(1.0);
  runs[primeCompiler].push_back(1.0);
  otherCompilers.erase(otherCompilers.begin());

  for (auto compiler : otherCompilers) {
    if (buildPrimeTime > 0.0) {
      builds[compiler].push_back(getTime(build.at(compiler)) / buildPrimeTime);
    }

    if (runPrimeTime > 0.0) {
      runs[compiler].push_back(getTime(run.at(compiler)) / runPrimeTime);
    }
  }
}

void compilerPerformanceReport(Sqlite3::DB &db, const StringList &compilers) {
  Sqlite3::DB::Results results;
  std::string currentSource;
  math::List testRuns;
  math::List sourceRuns;
  NamedMathList buildTrace, runTrace;
  NamedMathList buildPerf, runPerf;
  NamedMathList builds, runs;

  db.exec("SELECT "
          "source_identifier,compiler,trace_build_time,trace_run_time,build_"
          "time,run_time "
          "FROM run ORDER BY cast(source_identifier as text);",
          &results);

  for (auto row : results) {
    const std::string source =
        row("source_identifier", Sqlite3::TextType).text();
    const std::string compiler = row("compiler", Sqlite3::TextType).text();
    const double traceBuildTime =
        row("trace_build_time", Sqlite3::RealType).real();
    const double performanceBuildTime =
        row("build_time", Sqlite3::RealType).real();
    const double traceRunTime = row("trace_run_time", Sqlite3::RealType).real();
    const double performanceRunTime = row("run_time", Sqlite3::RealType).real();

    if (source != currentSource) {
      analyze(buildTrace, runTrace, builds, runs, compilers);
      analyze(buildPerf, runPerf, builds, runs, compilers);
      currentSource = source;
      buildTrace.clear();
      buildPerf.clear();
      runTrace.clear();
      runPerf.clear();
    }

    buildTrace[compiler].push_back(traceBuildTime);
    buildPerf[compiler].push_back(performanceBuildTime);
    runTrace[compiler].push_back(traceRunTime);
    runPerf[compiler].push_back(performanceRunTime);
  }
  analyze(buildTrace, runTrace, builds, runs, compilers);
  analyze(buildPerf, runPerf, builds, runs, compilers);

  printf("Compile Performace\n");
  for (auto compilerTimes : builds) {
    const double time = getTime(compilerTimes.second);

    printf("\t%s %0.2fx\n", compilerTimes.first.c_str(), time);
  }

  printf("Run Performace\n");
  for (auto compilerTimes : runs) {
    const double time = getTime(compilerTimes.second);

    printf("\t%s %0.2fx\n", compilerTimes.first.c_str(), time);
  }
}

/**
        @todo Evaluate performance of compile and run of various compilers
*/
int main(int argc, const char *const argv[]) {
  StringList testsToRun;
  StringList compilersToRun;
  io::Path openssl;
  String testNames;
  String testNamePrefix;
  const String testSuffix = "_test.cpp";
  bool testsPassed = false;
  const String compilers = String(",") + gCompilerList + String(",");

  // printf(ErrorTextFormatStart "Error" ClearTextFormat"\n");
  // printf(WarningTextFormatStart "Warning" ClearTextFormat"\n");
  // printf(BoldTextFormatStart "Bold" ClearTextFormat"\n");
  // printf(GoodTextFormatStart "Good" ClearTextFormat"\n");
  // printf(GreatTextFormatStart "Great" ClearTextFormat"\n");

  try {
    io::Path("tests").list(io::Path::NameOnly, testsToRun);
  } catch (const posix::err::ENOENT_Errno &) {
    printf(ErrorTextFormatStart "ERROR: Unable to find tests" ClearTextFormat
                                "\n");
  }
  for (auto i = testsToRun.begin(); i != testsToRun.end();) {
    if ((i->length() <= testSuffix.length()) ||
        (i->find(testSuffix) != i->length() - testSuffix.length())) {
      i = testsToRun.erase(i);
    } else {
      i->erase(i->length() - testSuffix.length());
      ++i;
    }
  }
  std::sort(testsToRun.begin(), testsToRun.end());
  for (int arg = 1; arg < argc; ++arg) {
    if (String("debug") == argv[arg]) {
      gDebugging = true;
    } else if (String("list") == argv[arg]) {
      for (auto test : testsToRun) {
        printf("%s\n", test.c_str());
      }
      testsToRun.clear();
    } else if (String("verbose") == argv[arg]) {
      gVerbose = true;
    } else if (String(argv[arg]).find("openssl=") == 0) {
      openssl = io::Path(String(argv[arg]).substr(8));
      if (!openssl.isDirectory()) {
        printf(
            WarningTextFormatStart
            "WARNING: %s is not a directory, disabling openssl" ClearTextFormat
            "\n",
            String(openssl).c_str());
        openssl = io::Path();
      } else {
        printf(BoldTextFormatStart
               "Enabling openssl with headers at: %s" ClearTextFormat "\n",
               String(openssl).c_str());
      }
    } else if (compilers.find(String(",") + String(argv[arg]) + String(",")) !=
               String::npos) {
      compilersToRun.push_back(argv[arg]);
    } else {
      const bool found =
          (io::Path("tests") + (String(argv[arg]) + "_test.cpp")).isFile();

      if (found) {
        if (!testsPassed) {
          testsPassed = true;
          testsToRun.clear();
        }
        testsToRun.push_back(argv[arg]);
      } else {
        printf("Compiler/Test not found: %s\n", argv[arg]);
      }
    }
  }
  try {
    String results;
    StringList headers;
    String parentDirectoryName =
        io::Path(argv[0]).canonical().parent().parent().name();
#if defined(__APPLE__)
    io::Path cacheDir = io::Path(env::get("HOME")) + "Library" + "Caches";
#else
    io::Path cacheDir = io::Path(env::get("HOME")) + ".testRuns";
#endif

    if (!cacheDir.isDirectory()) {
      cacheDir.mkdirs();
    }

    Sqlite3::DB db(cacheDir + (parentDirectoryName + "_tests.sqlite3"));

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
            "`timestamp` VARCHAR(20));");
    db.exec("CREATE TABLE IF NOT EXISTS `header` ("
            "`name` VARCHAR(256), "
            "`tests` TEXT, "
            "`options` TEXT, "
            "`hash` VARCHAR(32), "
            "`lines_run` INT, "
            "`code_lines` INT, "
            "`timestamp` VARCHAR(20));");

    exec::execute("mkdir -p bin/tests bin/logs", results);
    if (results != "") {
      printf(WarningTextFormatStart "WARNING: mkdir '%s'" ClearTextFormat "\n",
             results.c_str());
    }
    exec::execute(
        "rm -Rf *.gcov *.gcno *.gcda bin/coverage/* bin/logs/* bin/tests/*",
        results);
    if (results != "") {
      printf(WarningTextFormatStart "WARNING: rm '%s'" ClearTextFormat "\n",
             results.c_str());
    }
    std::string::size_type maxNameSize = 0;
    for (auto test : testsToRun) {
      if (test.size() > maxNameSize) {
        maxNameSize = test.size();
      }
    }
    if (compilersToRun.size() == 0) {
      split(gCompilerList, ',', compilersToRun);
    }
    for (auto test : testsToRun) {
      runTest(test, compilersToRun, maxNameSize, openssl, db);
      testNames = testNames + testNamePrefix + test;
      if (testNamePrefix.length() == 0) {
        testNamePrefix = ",";
      }
    }
    if (testsToRun.size() > 0) {
      printf("Examining overall coverage ...\n");
      exec::execute("ls *.h", results);
      split(results, '\n', headers);
      for (auto header : headers) {
        uint32_t coverage;
        uint32_t uncovered;
        StringList uncoveredLines;
        int expectedLinesRun = 0, expectedLinesNotRun = 0, bestCoverage;
        std::string headerHash;

        hashFile(header, headerHash);
        getHeaderStats(db, header, headerHash,
                       openssl.isEmpty() ? "" : "openssl", expectedLinesRun,
                       expectedLinesNotRun, testNames, bestCoverage);
        findFileCoverage(header, openssl.isEmpty() ? "" : "openssl", coverage,
                         uncovered, uncoveredLines, testNames, db);

        const bool hasExpectations =
            (expectedLinesRun > 0) || (expectedLinesNotRun > 0);
        const bool hasCoverage = (coverage > 0) || (uncovered > 0);
        const bool unexpectedCoverage =
            (coverage != uint32_t(expectedLinesRun));
        const bool unexpectedLines =
            (uncovered != uint32_t(expectedLinesNotRun));
        const int coverageRate =
            hasCoverage ? 100 * coverage / (coverage + uncovered) : 0;

        if ((hasExpectations || hasCoverage) &&
            (unexpectedCoverage || unexpectedLines)) {
          printf(WarningTextFormatStart "%s coverage changed %d/%d -> %d/%d "
                                        "(%d%% -> %d%%)" ClearTextFormat "\n",
                 header.c_str(), expectedLinesRun,
                 expectedLinesRun + expectedLinesNotRun, coverage,
                 coverage + uncovered,
                 hasExpectations ? 100 * expectedLinesRun /
                                       (expectedLinesRun + expectedLinesNotRun)
                                 : 0,
                 coverageRate);
          for (auto i : uncoveredLines) {
            printf("%s\n", i.c_str());
          }
        }
        if (coverageRate < bestCoverage) {
          printf(ErrorTextFormatStart "%s coverage is lower than best %d/%d "
                                      "(%d%%) < %d%%" ClearTextFormat "\n",
                 header.c_str(), coverage, coverage + uncovered, coverageRate,
                 bestCoverage);
          for (auto i : uncoveredLines) {
            printf("%s\n", i.c_str());
          }
        }
      }
    }

    performanceReport(db, testsToRun);
    if (compilersToRun.size() > 1) {
      compilerPerformanceReport(db, compilersToRun);
    }

  } catch (const std::exception &exception) {
    printf("EXCEPTION: %s\n", exception.what());
  }
  return 0;
}
