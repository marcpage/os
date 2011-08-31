#include "os/Exception.h"
#include "os/Sqlite3Plus.h"
#include <stdio.h>
#include <unistd.h>

// g++ -o /tmp/test Dropbox/posix/tests/Sqlite3_test.cpp -IDropbox/posix -lsqlite3 -g -Wall -Weffc++ -Wextra -Wshadow -Wwrite-strings
// /tmp/test

static void displayResults(const Sqlite3::DB::Results &results) {
	int	result= 1;

	for(Sqlite3::DB::Results::const_iterator	row= results.begin(); row != results.end(); ++row, ++result) {
		printf("Result #%d\n", result);
		for(Sqlite3::DB::Row::const_iterator column= row->begin(); column != row->end(); ++column) {
			printf("\t%s=%s\n", column->first.c_str(), column->second.c_str());
		}
	}
}

int main(int /*argc*/, const char * /*argv*/[]) {
	const char * const	kDBPath= "/tmp/test.sqlite3";

	try	{
		std::string	number;

		try	{
			errnoAssertPositiveMessageException(unlink(kDBPath)); // try to delete it
		} catch(const std::exception &exception) {
		}

		Sqlite3::DB				db(kDBPath);
		Sqlite3::DB::Row		row;
		Sqlite3::DB::Results	results;

		db.exec("CREATE TABLE `storage` ("
					"`id` INTEGER NOT NULL, "
					"`sha256` VARCHAR(64) NOT NULL, "
					"`accesses` INTEGER NOT NULL, "
					"`compressed` TINYINT NOT NULL, "
					"`created` VARCHAR(19) NOT NULL, " // 2010/01/01:22:02:01
					"`accessed` VARCHAR(19) NOT NULL, " // 2010/01/01:22:02:01
					"`payload` BLOB NOT NULL, "
					"`size` INTEGER NOT NULL, "
					"PRIMARY KEY ( `id` ));");
		row["sha256"]= "1234567890";
		row["accesses"]= "1";
		row["compressed"]= "0";
		row["created"]= "2010/03/02:10:16:00";
		row["accessed"]= "2010/03/02:10:16:00";
		row["payload"]= "Testing storing a row in the table";
		row["size"]= Sqlite3::toString(row["payload"].size(), number);
		db.addRow("storage", row);
		row["accesses"]= "2";
		row["compressed"]= "1";
		row["created"]= "2011/03/02:10:16:00";
		row["accessed"]= "2011/03/02:10:16:00";
		row["payload"]= "Testing a duplicate sha256";
		row["size"]= Sqlite3::toString(row["payload"].size(), number);
		db.addRow("storage", row);
		db.exec("SELECT * FROM `storage` WHERE `sha256` LIKE '1234567890';", &results);
		displayResults(results);
		db.exec("SELECT * FROM `storage` WHERE `id` = '1';", &results);
		displayResults(results);
		db.exec("SELECT * FROM `storage` WHERE `id` = '2';", &results);
		displayResults(results);
	} catch(const std::exception &exception) {
		printf("FAILED: Exception: %s\n", exception.what());
	}
	return 0;
}
