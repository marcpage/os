#include "os/Exception.h"
#include "os/POSIXErrno.h"
#include "os/Sqlite3Plus.h"
#include <stdio.h>
#include <unistd.h>

// g++ -o /tmp/test Dropbox/posix/tests/Sqlite3_test.cpp -IDropbox/posix
// -lsqlite3 -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings /tmp/test

static void displayResults(const Sqlite3::DB::Results &results) {
  int result = 1;

  for (auto row : results) {
    printf("Result #%d\n", result);
    for (auto column : row) {
      printf("\t%s=%s\n", column.name().c_str(),
             column.convertTo(Sqlite3::TextType).text().c_str());
    }
    ++result;
  }
}

int main(int argc, const char *const argv[]) {
  const char *const kDBPath = argc < 2 ? "bin/logs/test.sqlite3" : argv[1];

  try {
    std::string number;

    try {
      ErrnoOnNegative(unlink(kDBPath)); // try to delete it
    } catch (const std::exception &exception) {
    }

    Sqlite3::DB db(kDBPath);
    Sqlite3::Row row;
    Sqlite3::DB::Results results;
    Sqlite3::Value testValue(1, "integer"), *ptr;

    ptr = &testValue;
    testValue = *ptr;

    if (testValue.integer() != 1) {
      printf("FAIL: testValue != 1\n");
    }

    testValue = Sqlite3::Value(1.0, "real");

    if (testValue.real() != 1.0) {
      printf("FAIL: testValue != 1.0\n");
    }

    if (!Sqlite3::Value(Sqlite3::TextType).is(Sqlite3::TextType)) {
      printf("FAIL: TextType is not TextType\n");
    }

    if (!Sqlite3::Value(Sqlite3::IntegerType).is(Sqlite3::IntegerType)) {
      printf("FAIL: IntegerType is not IntegerType\n");
    }

    if (!Sqlite3::Value(Sqlite3::RealType).is(Sqlite3::RealType)) {
      printf("FAIL: RealType is not RealType\n");
    }

    if (!Sqlite3::Value(Sqlite3::BlobType).is(Sqlite3::BlobType)) {
      printf("FAIL: BlobType is not BlobType\n");
    }

    if (!Sqlite3::Value(Sqlite3::NullType).is(Sqlite3::NullType)) {
      printf("FAIL: NullType is not NullType\n");
    }

    if (Sqlite3::Value("4").convertTo(Sqlite3::IntegerType).integer() != 4) {
      printf("FAIL: convert('4') -> 4\n");
    }

    if (Sqlite3::Value(4).convertTo(Sqlite3::TextType).text() != "4") {
      printf("FAIL: convert(4) -> '4'\n");
    }

    if (Sqlite3::Value(Sqlite3::BlobType).blob("test", 4).blob() != "test") {
      printf("FAIL: 'test' -> blob\n");
    }

    if ((Sqlite3::Value(Sqlite3::RealType) = 4.5).real() != 4.5) {
      printf("FAIL: 4.5 -> 4.5\n");
    }

    if (Sqlite3::Value(4).convertTo(Sqlite3::BlobType).blob() != "4") {
      printf("FAIL: convert(4) -> Blob '4' vs '%s'\n",
             Sqlite3::Value(4).convertTo(Sqlite3::BlobType).blob().c_str());
    }

    if (Sqlite3::Value(4).convertTo(Sqlite3::RealType).real() != 4.0) {
      printf("FAIL: convert(4) -> 4.0\n");
    }

    if (atof(Sqlite3::Value(4.5).convertTo(Sqlite3::TextType).text().c_str()) !=
        4.5) {
      printf(
          "FAIL: convert(4.5) -> '4.5' vs '%s' vs %0.4f\n",
          Sqlite3::Value(4.5).convertTo(Sqlite3::TextType).text().c_str(),
          atof(
              Sqlite3::Value(4.5).convertTo(Sqlite3::TextType).text().c_str()));
    }

    if (atof(Sqlite3::Value(4.5).convertTo(Sqlite3::BlobType).blob().c_str()) !=
        4.5) {
      printf("FAIL: convert(4.5) -> Blob '4.5'\n");
    }

    if (Sqlite3::Value(4.0).convertTo(Sqlite3::IntegerType).integer() != 4) {
      printf("FAIL: convert(4.0) -> 4\n");
    }

    if (Sqlite3::Value("4.5").convertTo(Sqlite3::RealType).real() != 4.5) {
      printf("FAIL: convert('4.5') -> 4.5\n");
    }

    if (Sqlite3::Value(4.5).convertTo(Sqlite3::RealType).real() != 4.5) {
      printf("FAIL: convert(4.5) -> 4.5\n");
    }

    if (!Sqlite3::Value("4.5")
             .convertTo(Sqlite3::BlobType)
             .is(Sqlite3::BlobType)) {
      printf("FAIL: convert('4.5') to Blob is Blob\n");
    }

    if (!Sqlite3::Value()
             .blob("4.5")
             .convertTo(Sqlite3::BlobType)
             .is(Sqlite3::BlobType)) {
      printf("FAIL: blob('4.5') to Blob is Blob\n");
    }

    if (!Sqlite3::Value("test")
             .convertTo(Sqlite3::NullType)
             .is(Sqlite3::NullType)) {
      printf("FAIL: 'test' convert to null is not null\n");
    }

    if (atof(Sqlite3::Value()
                 .blob("4.5")
                 .convertTo(Sqlite3::TextType)
                 .text()
                 .c_str()) != 4.5) {
      printf("FAIL: blob('4.5') to Text is '4.5'\n");
    }

    if (Sqlite3::Value().blob("4.5").convertTo(Sqlite3::RealType).real() !=
        4.5) {
      printf("FAIL: blob('4.5') to Real is 4.5\n");
    }

    if (Sqlite3::Value().blob("4").convertTo(Sqlite3::IntegerType).integer() !=
        4) {
      printf("FAIL: blob('4') to Integer is 4\n");
    }

    if (!(Sqlite3::Value("text") = 1.0).is(Sqlite3::RealType)) {
      printf("FAIL: 'text' -> 1.0 is not Real\n");
    }

    if (!(Sqlite3::Value("text") = 1).is(Sqlite3::IntegerType)) {
      printf("FAIL: 'text' -> 1 is not Integer\n");
    }

    if (!(Sqlite3::Value(Sqlite3::BlobType) = "text").is(Sqlite3::BlobType)) {
      printf("FAIL: Blob -> 'text' is not Blob\n");
    }

    if (Sqlite3::Value().convertTo(Sqlite3::TextType).text() != "NULL") {
      printf("FAIL: convert(null) to Text -> '%s'\n",
             Sqlite3::Value().convertTo(Sqlite3::TextType).text().c_str());
    }

    if (!Sqlite3::Value().convertTo(Sqlite3::NullType).is(Sqlite3::NullType)) {
      printf("FAIL: null does not convert to null\n");
    }

    testValue = Sqlite3::Value(2.0);

    if (testValue.name() != "real") {
      printf("FAIL: testValue.name() != 'real'\n");
    }

    try {
      Sqlite3::Value(1).real();
      printf("FAIL: Value(1).real()\n");
    } catch (const Sqlite3::Exception &) {
    }

    try {
      Sqlite3::Value(1.0).integer();
      printf("FAIL: Value(1.0).integer()\n");
    } catch (const Sqlite3::Exception &) {
    }

    db.exec("CREATE TABLE `storage` ("
            "`id` INTEGER NOT NULL, "
            "`sha256` VARCHAR(64) NOT NULL, "
            "`accesses` INTEGER NOT NULL, "
            "`compressed` TINYINT NOT NULL, "
            "`created` VARCHAR(19) NOT NULL, "  // 2010/01/01:22:02:01
            "`accessed` VARCHAR(19) NOT NULL, " // 2010/01/01:22:02:01
            "`payload` BLOB NOT NULL, "
            "`size` INTEGER NOT NULL, "
            "`pi` REAL, "
            "`null` INTEGER, "
            "`blob` BLOB, "
            "PRIMARY KEY ( `id` ));");
    row["sha256"] = "1234567890abcdef";
    row("accesses", Sqlite3::IntegerType) = "1";
    row["compressed"] = 0;
    row["created"] = "2010/03/02:10:16:00";
    row["accessed"] = "2010/03/02:10:16:00";
    row["payload"] = "Testing storing a row in the table";
    row["size"] =
        std::to_string(row("payload", Sqlite3::TextType).text().size());
    db.addRow("storage", row);
    row["accesses"] = 2;
    row["compressed"] = 1;
    row["created"] = "2011/03/02:10:16:00";
    row["accessed"] = "2011/03/02:10:16:00";
    row["payload"] = "Testing a duplicate sha256";
    row["size"] =
        std::to_string(row("payload", Sqlite3::TextType).text().size());
    row["pi"] = 3.141592657;
    row["null"] = Sqlite3::Value(Sqlite3::NullType);
    row["blob"] = Sqlite3::Value().blob("blob");
    db.addRow("storage", row);
    db.exec("SELECT * FROM `storage` WHERE `sha256` LIKE '1234567890';",
            &results);
    displayResults(results);
    db.exec("SELECT * FROM `storage` WHERE `id` = '1';", &results);
    displayResults(results);
    db.exec("SELECT * FROM `storage` WHERE `id` = '2';", &results);
    displayResults(results);
#ifndef __Tracer_h__
    for (int i = 0; i < 2000; ++i) {
      db.addRow("storage", row);
    }
#endif
  } catch (const std::exception &exception) {
    printf("FAILED: Exception: %s\n", exception.what());
  }
  return 0;
}
