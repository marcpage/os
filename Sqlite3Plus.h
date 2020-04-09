#ifndef __Sqlite3Plus_h__
#define __Sqlite3Plus_h__

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include "os/Exception.h"

// http://www.iphonesdkarticles.com/2009/02/sqlite-tutorial-saving-images-in.html
// http://shellbt.wordpress.com/2009/07/14/sqlite3-blob-cecia-ucaauua/
// blob and sqlite3

/// Throws an Sqlite3::Exception for the given line
#define Sql3Throw() \
	throw Sqlite3::Exception(__FILE__, __LINE__)
	/// Throws an Sqlite3::Exception if a pointer is NULL
#define Sql3ThrowIfNull(ptr, message) \
	if(NULL == (ptr)) \
	throw Sqlite3::Exception(__FILE__, __LINE__, message); \
	else noop()
/// Throws an Sqlite3::Exception if 'error' is not NULL, 0, false, etc. 'error' must be a const char* or int
#define Sql3ThrowIfError(error) \
	if(error) \
	throw Sqlite3::Exception(__FILE__, __LINE__, error); \
	else noop()
/// Throws an Sqlite3::Exception if a sqlite3 return code is an error, throws the error string from sqlite3 for the db
#define Sql3ThrowIfDbError(db, returnCode) \
	if(returnCode) \
	throw Sqlite3::Exception(__FILE__, __LINE__, sqlite3_errmsg(db)); \
	else noop()
/// Throws an Sqlite3::Exception if an assertion fails
#define Sql3Assert(condition) \
	if(!(condition)) \
	throw Sqlite3::Exception(__FILE__, __LINE__, #condition); \
	else noop()

/**
	@todo Test!
*/
namespace Sqlite3 {

	inline void noop() {}

	std::string &escapeValue(std::string &data);
	template<typename Number>
	std::string &toString(Number i, std::string &asString);
	template<typename Number>
	Number fromString(const std::string &asString);

	/// exceptions in Sqlite3
	class Exception : public msg::Exception {
	public:
		/// copy constructor
		Exception(const Exception &exception) throw();
		/** @brief construct an exception for the given file and line number
		 @param file	The file in which the exception occurred
		 @param line	The line number at which the exception occurred
		 */
		Exception(const char *file, int line);
		/** @brief construct an exception for the given file, line number and an error code
		 @param file		The file in which the exception occurred
		 @param line		The line number at which the exception occurred
		 @param errorCode	An error code
		 */
		Exception(const char *file, int line, int errorCode);
		/** @brief construct an exception for the given file, line number and an error message
		 @param file		The file in which the exception occurred
		 @param line		The line number at which the exception occurred
		 @param errorString	A message describing the error
		 */
		Exception(const char *file, int line, const std::string &errorString);
		/// destructor
		// cppcheck-suppress missingOverride
		virtual ~Exception() throw();
		/// assignment operator
		Exception &operator=(const Exception &exception);
	};

	/// An Sqlite3 database
	class DB {
	public:
		typedef std::string				String;
		typedef std::map<String,String>	Row;
		typedef std::vector<Row>		Results;
		/** @brief Opens or Creates a database at the given path
		 @throw Sqlite3::Exception	If there is any problem executing the query
		 */
		explicit DB(const String &filepath);
		/// Closes and cleans up the database
		~DB();
		/** @brief Executes an sql query
		 @param command	The command to execute
		 @param rows	Must be allocated before the call.
						Filled with Rows of items describing the results
		 @throw Sqlite3::Exception	If there is any problem executing the query
		 */
		void exec(const String &command, Results *rows= NULL);
		/** @brief Adds a row to the given table.
			@param table	The name of the table to add the row to
			@param row		The data for the row to add
			@throw Sqlite3::Exception	If there is any problem adding the row
		*/
		void addRow(const String &table, const Row &row);
	private:
		sqlite3					*_db;	///< The database reference

		DB(const DB&); ///< cannot be used
		DB &operator=(const DB&); ///< cannot be used
	};
}

#include <stdarg.h>
#include <sstream>

namespace Sqlite3 {
	inline std::string &escapeValue(std::string &data) {
		std::string::size_type	tick= data.find('\'');

		while(std::string::npos != tick) {
			data.replace(tick, 1, "''");
			tick= data.find('\'', tick + 2);
		}
		return data;
	}
	template<typename Number>
	std::string &toString(Number i, std::string &asString) {
		std::stringstream	ss;

		ss << i;
		asString= ss.str();
		return asString;
	}
	template<typename Number>
	Number fromString(const std::string &asString) {
		std::istringstream	i(asString);
		Number				x;

		if( !(i >> x) ) {
			throw Sqlite3::Exception(__FILE__, __LINE__, "Unable to convert number string to number");
		}
		return x;
	}

	// ****** Exception implementation ******

	inline Exception::Exception(const Exception &exceptionToCopy) throw()
		:msg::Exception(exceptionToCopy) {}
	inline Exception::Exception(const char *file, int line)
		:msg::Exception("Error", file, line) {
	}
	inline Exception::Exception(const char *file, int line, int errorCode)
		:msg::Exception("Error #"+std::to_string(errorCode), file, line) {
	}
	inline Exception::Exception(const char *file, int line, const std::string &errorString)
		:msg::Exception("Error "+errorString, file, line) {
	}
	inline Exception::~Exception() throw() {}
	inline Exception &Exception::operator=(const Exception &exceptionToCopy) {
		*reinterpret_cast<msg::Exception*>(this)= exceptionToCopy;
		return *this;
	}

	// ****** DB implementation ******

	inline DB::DB(const String &filepath)
		:_db(NULL) {
		Sql3ThrowIfDbError(_db, sqlite3_open(filepath.c_str(), &_db));
	}
	inline DB::~DB() {
		if(sqlite3_close(_db)) {
			// error, but what can we do?
		}
		_db= NULL;
	}
	inline void DB::exec(const String &command, Results *rows) {
		sqlite3_stmt	*statement= NULL;
		int				stepResult;

		if(NULL != rows) {
			rows->clear();
		}
		Sql3ThrowIfDbError(_db, sqlite3_prepare_v2(_db, command.c_str(), command.length(), &statement, NULL));

		do	{
			stepResult= sqlite3_step(statement);
			if(SQLITE_ROW == stepResult) {
				if(NULL != rows) {
					const int	columns= sqlite3_column_count(statement);
					Row		row;
					String	key;

					for(int column= 0; (column < columns); ++column) {
						const int	columnSize= sqlite3_column_bytes(statement, column);

						key= sqlite3_column_name(statement, column);
						row[key].assign(reinterpret_cast<const char*>(sqlite3_column_blob(statement, column)), columnSize);
					}
					rows->push_back(row);
				}
			} else if(SQLITE_DONE != stepResult) {
				Sql3ThrowIfDbError(_db, sqlite3_finalize(statement));
				Sql3ThrowIfDbError(_db, stepResult);
			}
		} while(SQLITE_DONE != stepResult);
		Sql3ThrowIfDbError(_db, sqlite3_finalize(statement));
	}
	void DB::addRow(const String &table, const Row &row) {
		sqlite3_stmt		*statement= NULL;
		String				command("INSERT INTO `");
		String				values;
		char				separator= ' ';
		std::vector<String>	keys;

		command.append(table).append("` (");
		for(Row::const_iterator column= row.begin(); (column != row.end()); ++column) {
			command.append(1, separator).append(1, '`').append(column->first).append(1, '`');
			values.append(1, separator).append(1, '?');
			keys.push_back(column->first);
			separator= ',';
		}
		command.append(") VALUES (").append(values).append(");");
		Sql3ThrowIfDbError(_db, sqlite3_prepare_v2(_db, command.c_str(), command.length(), &statement, NULL));
		int	valueIndex= 1;
		for(std::vector<String>::iterator	keyPtr= keys.begin(); (keyPtr != keys.end()); ++keyPtr, ++valueIndex) {
			const String	&value= row.find(*keyPtr)->second;

			sqlite3_bind_blob(statement, valueIndex, value.data(), value.size(), SQLITE_TRANSIENT);
		}
		Sql3Assert(SQLITE_DONE == sqlite3_step(statement));
		Sql3ThrowIfDbError(_db, sqlite3_finalize(statement));
	}
}

#endif // __Sqlite3Plus_h__

/*


int sqlite3_column_bytes(sqlite3_stmt*, int iCol);
const void *sqlite3_column_blob(sqlite3_stmt*, int iCol);
const char *sqlite3_column_name(sqlite3_stmt*, int N);
int sqlite3_column_count(sqlite3_stmt *pStmt);

int sqlite3_prepare_v2(
  sqlite3 *db,            /¥ Database handle ¥/
  const char *zSql,       /¥ SQL statement, UTF-8 encoded ¥/
  int nByte,              /¥ Maximum length of zSql in bytes. ¥/
  sqlite3_stmt **ppStmt,  /¥ OUT: Statement handle ¥/
  const char **pzTail     /¥ OUT: Pointer to unused portion of zSql ¥/
);

To execute an SQL query, it must first be compiled into a byte-code program using one of these
	routines.
The first argument, "db", is a database connection obtained from a prior successful call to
	sqlite3_open(), sqlite3_open_v2() or sqlite3_open16(). The database connection must not have
	been closed.
The second argument, "zSql", is the statement to be compiled, encoded as either UTF-8 or UTF-16.
	The sqlite3_prepare() and sqlite3_prepare_v2() interfaces use UTF-8, and sqlite3_prepare16()
		and sqlite3_prepare16_v2() use UTF-16.
If the nByte argument is less than zero, then zSql is read up to the first zero terminator. If nByte
	is non-negative, then it is the maximum number of bytes read from zSql. When nByte is
	non-negative, the zSql string ends at either the first '\000' or '\u0000' character or the
	nByte-th byte, whichever comes first. If the caller knows that the supplied string is
	nul-terminated, then there is a small performance advantage to be gained by passing an nByte
	parameter that is equal to the number of bytes in the input string including the nul-terminator
	bytes.
If pzTail is not NULL then *pzTail is made to point to the first byte past the end of the first SQL
	statement in zSql. These routines only compile the first statement in zSql, so *pzTail is left
	pointing to what remains uncompiled.
*ppStmt is left pointing to a compiled prepared statement that can be executed using sqlite3_step().
	If there is an error, *ppStmt is set to NULL. If the input text contains no SQL (if the input is
	an empty string or a comment) then *ppStmt is set to NULL. The calling procedure is responsible
	for deleting the compiled SQL statement using sqlite3_finalize() after it has finished with it.
	ppStmt may not be NULL.
On success, the sqlite3_prepare() family of routines return SQLITE_OK; otherwise an error code is
	returned.
The sqlite3_prepare_v2() and sqlite3_prepare16_v2() interfaces are recommended for all new programs.
	The two older interfaces are retained for backwards compatibility, but their use is discouraged.
	In the "v2" interfaces, the prepared statement that is returned (the sqlite3_stmt object)
	contains a copy of the original SQL text. This causes the sqlite3_step() interface to behave
	differently in three ways:

	1.	If the database schema changes, instead of returning SQLITE_SCHEMA as it always used to do,
		sqlite3_step() will automatically recompile the SQL statement and try to run it again. If
		the schema has changed in a way that makes the statement no longer valid, sqlite3_step()
		will still return SQLITE_SCHEMA. But unlike the legacy behavior, SQLITE_SCHEMA is now a
		fatal error. Calling sqlite3_prepare_v2() again will not make the error go away. Note: use
		sqlite3_errmsg() to find the text of the parsing error that results in an SQLITE_SCHEMA
		return.
	2.	When an error occurs, sqlite3_step() will return one of the detailed error codes or extended
		error codes. The legacy behavior was that sqlite3_step() would only return a generic
		SQLITE_ERROR result code and the application would have to make a second call to
		sqlite3_reset() in order to find the underlying cause of the problem. With the "v2" prepare
		interfaces, the underlying reason for the error is returned immediately.
	5.	If the value of a host parameter in the WHERE clause might change the query plan for a
		statement, then the statement may be automatically recompiled (as if there had been a schema
		change) on the first sqlite3_step() call following any change to the bindings of the
		parameter.


int sqlite3_step(sqlite3_stmt*);

After a prepared statement has been prepared using either sqlite3_prepare_v2() or
	sqlite3_prepare16_v2() or one of the legacy interfaces sqlite3_prepare() or sqlite3_prepare16(),
	this function must be called one or more times to evaluate the statement.
The details of the behavior of the sqlite3_step() interface depend on whether the statement was
	prepared using the newer "v2" interface sqlite3_prepare_v2() and sqlite3_prepare16_v2() or the
	older legacy interface sqlite3_prepare() and sqlite3_prepare16(). The use of the new "v2"
	interface is recommended for new applications but the legacy interface will continue to be
	supported.
In the legacy interface, the return value will be either SQLITE_BUSY, SQLITE_DONE, SQLITE_ROW,
	SQLITE_ERROR, or SQLITE_MISUSE. With the "v2" interface, any of the other result codes or
	extended result codes might be returned as well.
SQLITE_BUSY means that the database engine was unable to acquire the database locks it needs to do
	its job. If the statement is a COMMIT or occurs outside of an explicit transaction, then you can
	retry the statement. If the statement is not a COMMIT and occurs within a explicit transaction
	then you should rollback the transaction before continuing.
SQLITE_DONE means that the statement has finished executing successfully. sqlite3_step() should not
	be called again on this virtual machine without first calling sqlite3_reset() to reset the
	virtual machine back to its initial state.
If the SQL statement being executed returns any data, then SQLITE_ROW is returned each time a new
	row of data is ready for processing by the caller. The values may be accessed using the column
	access functions. sqlite3_step() is called again to retrieve the next row of data.
SQLITE_ERROR means that a run-time error (such as a constraint violation) has occurred.
	sqlite3_step() should not be called again on the VM. More information may be found by calling
	sqlite3_errmsg(). With the legacy interface, a more specific error code (for example,
	SQLITE_INTERRUPT, SQLITE_SCHEMA, SQLITE_CORRUPT, and so forth) can be obtained by calling
	sqlite3_reset() on the prepared statement. In the "v2" interface, the more specific error code
	is returned directly by sqlite3_step().
SQLITE_MISUSE means that the this routine was called inappropriately. Perhaps it was called on a
	prepared statement that has already been finalized or on one that had previously returned
	SQLITE_ERROR or SQLITE_DONE. Or it could be the case that the same database connection is being
	used by two or more threads at the same moment in time.
Goofy Interface Alert: In the legacy interface, the sqlite3_step() API always returns a generic
	error code, SQLITE_ERROR, following any error other than SQLITE_BUSY and SQLITE_MISUSE. You must
	call sqlite3_reset() or sqlite3_finalize() in order to find one of the specific error codes that
	better describes the error. We admit that this is a goofy design. The problem has been fixed
	with the "v2" interface. If you prepare all of your SQL statements using either
	sqlite3_prepare_v2() or sqlite3_prepare16_v2() instead of the legacy sqlite3_prepare() and
	sqlite3_prepare16() interfaces, then the more specific error codes are returned directly by
	sqlite3_step(). The use of the "v2" interface is recommended.



int sqlite3_reset(sqlite3_stmt *pStmt);

The sqlite3_reset() function is called to reset a prepared statement object back to its initial
	state, ready to be re-executed. Any SQL statement variables that had values bound to them using
	the sqlite3_bind_*() API retain their values. Use sqlite3_clear_bindings() to reset the bindings.
The sqlite3_reset(S) interface resets the prepared statement S back to the beginning of its program.
If the most recent call to sqlite3_step(S) for the prepared statement S returned SQLITE_ROW or
	SQLITE_DONE, or if sqlite3_step(S) has never before been called on S, then sqlite3_reset(S)
	returns SQLITE_OK.
If the most recent call to sqlite3_step(S) for the prepared statement S indicated an error, then
	sqlite3_reset(S) returns an appropriate error code.
The sqlite3_reset(S) interface does not change the values of any bindings on the prepared statement
	S.



int sqlite3_finalize(sqlite3_stmt *pStmt);

The sqlite3_finalize() function is called to delete a prepared statement. If the statement was
	executed successfully or not executed at all, then SQLITE_OK is returned. If execution of the
	statement failed then an error code or extended error code is returned.
This routine can be called at any point during the execution of the prepared statement. If the
	virtual machine has not completed execution when this routine is called, that is like
	encountering an error or an interrupt. Incomplete updates may be rolled back and transactions
	canceled, depending on the circumstances, and the error code returned will be SQLITE_ABORT.



int sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*)); SQLITE_TRANSIENT


*/
