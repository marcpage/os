#ifndef __Sqlite3Plus_h__
#define __Sqlite3Plus_h__

#include "os/Exception.h"
#include <map>
#include <sqlite3.h>
#include <string>
#include <vector>

// http://www.iphonesdkarticles.com/2009/02/sqlite-tutorial-saving-images-in.html
// http://shellbt.wordpress.com/2009/07/14/sqlite3-blob-cecia-ucaauua/
// blob and sqlite3

/// Throws an Sqlite3::Exception if a pointer is NULL
#define Sql3ThrowIfNull(ptr, message)                                          \
  if (NULL == (ptr))                                                           \
    throw Sqlite3::Exception(__FILE__, __LINE__, message);                     \
  else                                                                         \
    noop()
/// Throws an Sqlite3::Exception if 'error' is not NULL, 0, false, etc. 'error'
/// must be a const char* or int
#define Sql3ThrowIfError(error)                                                \
  if (error)                                                                   \
    throw Sqlite3::Exception(__FILE__, __LINE__, error);                       \
  else                                                                         \
    noop()
/// Throws an Sqlite3::Exception with a message
#define Sql3ThrowMessage(message)                                              \
  throw Sqlite3::Exception(__FILE__, __LINE__, message);
/// Throws an Sqlite3::Exception if a sqlite3 return code is an error, throws
/// the error string from sqlite3 for the db
#define Sql3ThrowIfDbError(db, returnCode)                                     \
  if (returnCode)                                                              \
    throw Sqlite3::Exception(__FILE__, __LINE__, sqlite3_errmsg(db));          \
  else                                                                         \
    noop()
/// Throws an Sqlite3::Exception if an assertion fails
#define Sql3Assert(condition)                                                  \
  if (!(condition))                                                            \
    throw Sqlite3::Exception(__FILE__, __LINE__, #condition);                  \
  else                                                                         \
    noop()

namespace Sqlite3 {

/// Noop to support zlib_handle_error macro. Does nothing.
inline void noop() {}

/// Escapes a value to be passed to sqlite execute
std::string &escapeValue(std::string &data);
/// Converts a string to a number
template <typename Number> Number fromString(const std::string &asString);

/// exceptions in Sqlite3
class Exception : public msg::Exception {
public:
  /// copy constructor
  Exception(const Exception &exception) throw();
  /** @brief construct an exception for the given file, line number and an error
   code
   @param file		The file in which the exception occurred
   @param line		The line number at which the exception occurred
   @param errorCode	An error code
   */
  Exception(const char *file, int line, int errorCode);
  /** @brief construct an exception for the given file, line number and an error
   message
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

/// @todo Document
enum Type { NullType, TextType, IntegerType, RealType, BlobType };

class Value {
public:
  Value(Type t = NullType, const std::string &name = "")
      : _value(_create(t)), _name(name) {}
  Value(const void *buffer, size_t bytes, const std::string &name = "")
      : _value(new Blob(std::string(static_cast<const char *>(buffer), bytes))),
        _name(name) {}
  Value(sqlite3_stmt *statement, int column)
      : _value(_load(statement, column)),
        _name(sqlite3_column_name(statement, column)) {}
  explicit Value(int value, const std::string &name = "")
      : _value(new Integer(value)), _name(name) {}
  explicit Value(int64_t value, const std::string &name = "")
      : _value(new Integer(value)), _name(name) {}
  explicit Value(const std::string &value, const std::string &name = "")
      : _value(new Text(value)), _name(name) {}
  explicit Value(double value, const std::string &name = "")
      : _value(new Real(value)), _name(name) {}
  // cppcheck-suppress copyCtorPointerCopying
  Value(const Value &other) : _value(other._value), _name(other._name) {
    if (nullptr != other._value) {
      _value = other._value->clone();
    }
  }
  ~Value() { delete _value; }
  Value convertTo(Type t) const {
    if (is(NullType)) {
      if (TextType == t) {
        return Value(*this) = "NULL";
      }
      return Value(t, _name);
    }
    if (NullType == t) {
      return Value(NullType, _name);
    }
    if (is(t)) {
      return Value(*this);
    }
    return Value(_value->convertTo(t), _name);
  }
  Value &name(const std::string &name) {
    _name = name;
    return *this;
  }
  std::string &name() { return _name; }
  const std::string &name() const { return _name; }
  Value &make(Type t) {
    delete _value;
    _value = _create(t);
    return *this;
  }
  bool is(Type t) const { return type() == t; }
  Type type() const { return nullptr == _value ? NullType : _value->type(); }
  const Value &bind(sqlite3 *db, sqlite3_stmt *statement, int column) const {
    if (nullptr == _value) {
      Sql3ThrowIfDbError(db, sqlite3_bind_null(statement, column));
    } else {
      _value->bind(db, statement, column);
    }
    return *this;
  }
  int64_t integer() const {
    Sql3Assert(is(IntegerType));

    return _value->integer();
  }
  double real() const {
    Sql3Assert(is(RealType));

    return _value->real();
  }
  const std::string &text() const {
    Sql3Assert(is(TextType));

    return _value->data();
  }
  const std::string &blob() const {
    Sql3Assert(is(BlobType));

    return _value->data();
  }
  Value &blob(const void *data, size_t bytes);
  Value &blob(const std::string &data) {
    return blob(data.data(), data.size());
  }
  Value &operator=(const Value &other) {
    if (this == &other) {
      return *this;
    }

    delete _value;
    _value = nullptr == other._value ? nullptr : other._value->clone();

    if (other._name.size() > 0) {
      _name = other._name;
    }

    return *this;
  }
  Value &operator=(int value) {
    *this = int64_t(value);
    return *this;
  }
  Value &operator=(int64_t value);
  Value &operator=(double value);
  Value &operator=(const std::string &textOrData);
  Value &operator=(const char *text) { return operator=(std::string(text)); }

private:
  class Instance {
  public:
    Instance() {}
    virtual ~Instance() {}
    virtual Type type() = 0;
    virtual Instance *clone() = 0;
    virtual void bind(sqlite3 *db, sqlite3_stmt *statement, int column) = 0;
    virtual Instance *convertTo(Type t) = 0;
    virtual std::string &data() {                              // cannot test
      Sql3ThrowMessage("Wrong Type");                          // cannot test
    }                                                          // cannot test
    virtual double &real() { Sql3ThrowMessage("Wrong Type"); } // cannot test
    virtual int64_t &integer() {                               // cannot test
      Sql3ThrowMessage("Wrong Type");                          // cannot test
    }                                                          // cannot test
  };
  class Text : public Instance {
  public:
    Text(const std::string &value = "") : _value(value) {}
    Text(sqlite3_stmt *statement, int column)
        : _value(reinterpret_cast<const char *>(
                     sqlite3_column_text(statement, column)),
                 sqlite3_column_bytes(statement, column)) {}
    virtual ~Text() {}
    Type type() override { return TextType; }
    Instance *clone() override { return new Text(_value); }
    void bind(sqlite3 *db, sqlite3_stmt *statement, int column) override {
      Sql3ThrowIfDbError(db, sqlite3_bind_text(statement, column, _value.data(),
                                               _value.size(), nullptr));
    }
    Instance *convertTo(Type t) override {
      switch (t) {
      case IntegerType:
        return new Integer(::atol(_value.c_str()));
      case RealType:
        return new Real(::atof(_value.c_str()));
      case BlobType:
        return new Blob(_value);
      case NullType:
      case TextType:
      default:
        break; // not testable
      }
      Sql3ThrowMessage("Should not be able to get here");
      return nullptr;
    }
    std::string &data() override { return _value; }

  private:
    std::string _value;
  };
  class Blob : public Text {
  public:
    Blob(const std::string &value = "") : Text(value) {}
    Blob(sqlite3_stmt *statement, int column)
        : Text(std::string(
              static_cast<const char *>(sqlite3_column_blob(statement, column)),
              sqlite3_column_bytes(statement, column))) {}
    virtual ~Blob() {}
    Type type() override { return BlobType; }
    Instance *clone() override { return new Blob(data()); }
    void bind(sqlite3 *db, sqlite3_stmt *statement, int column) override {
      Sql3ThrowIfDbError(db,
                         sqlite3_bind_blob64(statement, column, data().data(),
                                             data().size(), SQLITE_TRANSIENT));
    }
    Instance *convertTo(Type t) override {
      switch (t) {
      case IntegerType:
        return new Integer(::atol(data().c_str()));
      case RealType:
        return new Real(::atof(data().c_str()));
      case TextType:
        return new Text(data());
      case NullType:
      case BlobType:
      default:
        break; // not testable
      }
      Sql3ThrowMessage("Should not be able to get here");
      return nullptr;
    }
  };
  class Integer : public Instance {
  public:
    Integer(int64_t value = 0) : _value(value) {}
    Integer(sqlite3_stmt *statement, int column)
        : _value(sqlite3_column_int64(statement, column)) {}
    virtual ~Integer() {}
    Type type() override { return IntegerType; }
    Instance *clone() override { return new Integer(_value); }
    int64_t &integer() override { return _value; }
    void bind(sqlite3 *db, sqlite3_stmt *statement, int column) override {
      Sql3ThrowIfDbError(db, sqlite3_bind_int64(statement, column, _value));
    }
    Instance *convertTo(Type t) override {
      switch (t) {
      case BlobType:
        return new Blob(std::to_string(_value));
      case RealType:
        return new Real(double(_value));
      case TextType:
        return new Text(std::to_string(_value));
      case IntegerType:
      case NullType:
      default:
        break; // not testable
      }
      Sql3ThrowMessage("Should not be able to get here");
      return nullptr;
    }

  private:
    int64_t _value;
  };
  class Real : public Instance {
  public:
    Real(double value = 0) : _value(value) {}
    Real(sqlite3_stmt *statement, int column)
        : _value(sqlite3_column_double(statement, column)) {}
    virtual ~Real() {}
    Type type() override { return RealType; }
    Instance *clone() override { return new Real(_value); }
    double &real() override { return _value; }
    void bind(sqlite3 *db, sqlite3_stmt *statement, int column) override {
      Sql3ThrowIfDbError(db, sqlite3_bind_double(statement, column, _value));
    }
    Instance *convertTo(Type t) override {
      switch (t) {
      case BlobType:
        return new Blob(std::to_string(_value));
      case IntegerType:
        return new Integer(int64_t(_value));
      case TextType:
        return new Text(std::to_string(_value));
      case RealType:
      case NullType:
      default:
        break; // not testable
      }
      Sql3ThrowMessage("Should not be able to get here");
      return nullptr;
    }

  private:
    double _value;
  };
  Value(Instance *value, const std::string &name)
      : _value(value), _name(name) {}
  Instance *_value;
  std::string _name;

  static Instance *_create(Type t) {
    switch (t) {
    case TextType:
      return new Text();
    case BlobType:
      return new Blob();
    case IntegerType:
      return new Integer();
    case RealType:
      return new Real();
    case NullType:
    default:
      break;
    }
    return nullptr;
  }
  static Instance *_load(sqlite3_stmt *statement, int column) {
    const int columnType = sqlite3_column_type(statement, column);
    Instance *blobValue;

    switch (columnType) {
    case SQLITE_INTEGER:
      return new Integer(statement, column);
    case SQLITE_FLOAT:
      return new Real(statement, column);
    case SQLITE_BLOB:
      return new Blob(statement, column);
    case SQLITE_TEXT:
      return new Text(statement, column);
    case SQLITE_NULL:
      blobValue = new Blob(statement, column);

      if (blobValue->data().size() == 0) {
        delete blobValue;
      } else {
        return blobValue; // null value actually had a value?
      }

      break;
    default:
      break; // not testable
    }
    return nullptr;
  }
};

inline Value &Value::blob(const void *data, size_t bytes) {
  if (is(BlobType)) {
    _value->data() = std::string(reinterpret_cast<const char *>(data), bytes);
  } else {
    delete _value;
    _value = new Blob(std::string(reinterpret_cast<const char *>(data), bytes));
  }
  return *this;
}

inline Value &Value::operator=(int64_t value) {
  if (is(IntegerType)) {
    _value->integer() = value;
  } else {
    delete _value;
    _value = new Integer(value);
  }
  return *this;
}

inline Value &Value::operator=(double value) {
  if (is(RealType)) {
    _value->real() = value;
  } else {
    delete _value;
    _value = new Real(value);
  }
  return *this;
}

inline Value &Value::operator=(const std::string &textOrData) {
  if (is(BlobType) || is(TextType)) {
    _value->data() = textOrData;
  } else {
    delete _value;
    _value = new Text(textOrData);
  }
  return *this;
}

class Row : public std::vector<Value> {
public:
  Row() : std::vector<Value>() {}
  explicit Row(const Value &value) : std::vector<Value>() { push_back(value); }
  ~Row() {}
  Value &operator()(const std::string &name, Type t) {
    return (*this)[name] = (*this)[name].convertTo(t);
  }
  Value &operator[](const std::string &name) {
    for (auto v = begin(); v != end(); ++v) {
      if (v->name() == name) {
        return *v;
      }
    }
    push_back(Value(NullType, name));
    return std::vector<Value>::operator[](size() - 1);
  }
  const Value &operator[](const std::string &name) const {
    for (auto v = begin(); v != end(); ++v) {
      if (v->name() == name) {
        return *v;
      }
    }
    Sql3ThrowMessage("Key not found");
    return std::vector<Value>::operator[](0);
  }
  Row &erase(const std::string &name) {
    for (auto v = begin(); v != end(); ++v) {
      if (v->name() == name) {
        std::vector<Value>::erase(v);
        break;
      }
    }
    return *this;
  }
  Row &operator<<(const Value &v) {
    push_back(v);
    return *this;
  }
};

inline Row operator<<(const Value &v1, const Value &v2) {
  return Row() << v1 << v2;
}

/// An Sqlite3 database
class DB {
public:
  typedef std::string String;       ///< Strings used
  typedef std::vector<Row> Results; ///< Rows that match a query
  /** @brief Opens or Creates a database at the given path
   @throw Sqlite3::Exception	If there is any problem executing the query
   */
  explicit DB(const String &filepath);
  /// Closes and cleans up the database
  ~DB();
  /** @brief Executes an sql query
   @param command	The command to execute
   @param rows	Must be allocated before the call.
                                  Filled with Rows of items describing the
   results
   @throw Sqlite3::Exception	If there is any problem executing the query
   */
  void exec(const String &command, Results *rows = NULL);
  /** @brief Adds a row to the given table.
          @param table	The name of the table to add the row to
          @param row		The data for the row to add
          @throw Sqlite3::Exception	If there is any problem adding the row
  */
  void addRow(const String &table, const Row &row);

private:
  sqlite3 *_db; ///< The database reference

  DB(const DB &);            ///< cannot be used
  DB &operator=(const DB &); ///< cannot be used
};
} // namespace Sqlite3

#include <sstream>
#include <stdarg.h>

namespace Sqlite3 {
/**
        Replaces all apostrophes with double apostrophe.
        @param data The value to escape. It will be modified
        @return a reference to data
*/
inline std::string &escapeValue(std::string &data) {
  auto tick = data.find('\'');

  while (std::string::npos != tick) {
    data.replace(tick, 1, "''");
    tick = data.find('\'', tick + 2);
  }
  return data;
}
/**
        @param asString the numeric value
        @return the number of type Number of the string
        @throws Sqlite3::Exception if the string is not convertible to the type.
*/
template <typename Number> Number fromString(const std::string &asString) {
  std::istringstream i(asString);
  Number x;

  if (!(i >> x)) {
    throw Sqlite3::Exception(__FILE__, __LINE__,
                             "Unable to convert number string to number");
  }
  return x;
}

// ****** Exception implementation ******

inline Exception::Exception(const Exception &exceptionToCopy) throw()
    : msg::Exception(exceptionToCopy) {}
inline Exception::Exception(const char *file, int line, int errorCode)
    : msg::Exception("Error #" + std::to_string(errorCode), file, line) {}
inline Exception::Exception(const char *file, int line,
                            const std::string &errorString)
    : msg::Exception("Error " + errorString, file, line) {}
inline Exception::~Exception() throw() {}
inline Exception &Exception::operator=(const Exception &exceptionToCopy) {
  *reinterpret_cast<msg::Exception *>(this) = exceptionToCopy;
  return *this;
}

// ****** DB implementation ******

inline DB::DB(const String &filepath) : _db(NULL) {
  Sql3ThrowIfDbError(_db, sqlite3_open(filepath.c_str(), &_db));
}
inline DB::~DB() {
  if (sqlite3_close(_db)) {
    // error, but what can we do?
  }
  _db = NULL;
}
inline void DB::exec(const String &command, Results *rows) {
  sqlite3_stmt *statement = NULL;
  int stepResult;

  if (NULL != rows) {
    rows->clear();
  }
  Sql3ThrowIfDbError(_db,
                     sqlite3_prepare_v2(_db, command.c_str(), command.length(),
                                        &statement, NULL));

  do {
    stepResult = sqlite3_step(statement);
    if (SQLITE_ROW == stepResult) {
      if (NULL != rows) {
        const auto columns = sqlite3_column_count(statement);
        Row row;

        for (auto column = 0; (column < columns); ++column) {
          row << Value(statement, column);
        }
        rows->push_back(row);
      }
    } else if (SQLITE_DONE != stepResult) {
      Sql3ThrowIfDbError(_db, sqlite3_finalize(statement)); // not tested
      Sql3ThrowIfDbError(_db, stepResult);                  // not tested
    }
  } while (SQLITE_DONE != stepResult);
  Sql3ThrowIfDbError(_db, sqlite3_finalize(statement));
}
void DB::addRow(const String &table, const Row &row) {
  sqlite3_stmt *statement = NULL;
  String command("INSERT INTO `");
  String values;
  auto separator = ' ';

  command.append(table).append("` (");

  for (auto column : row) {
    command.append(1, separator)
        .append(1, '`')
        .append(column.name())
        .append(1, '`');
    values.append(1, separator).append(1, '?');
    separator = ',';
  }

  command.append(") VALUES (").append(values).append(");");
  Sql3ThrowIfDbError(_db,
                     sqlite3_prepare_v2(_db, command.c_str(), command.length(),
                                        &statement, NULL));
  int valueIndex = 1;
  for (auto column = row.begin(); column != row.end(); ++column) {
    column->bind(_db, statement, valueIndex);
    ++valueIndex;
  }
  Sql3Assert(SQLITE_DONE == sqlite3_step(statement));
  Sql3ThrowIfDbError(_db, sqlite3_finalize(statement));
}
} // namespace Sqlite3

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

To execute an SQL query, it must first be compiled into a byte-code program
using one of these routines. The first argument, "db", is a database connection
obtained from a prior successful call to sqlite3_open(), sqlite3_open_v2() or
sqlite3_open16(). The database connection must not have been closed. The second
argument, "zSql", is the statement to be compiled, encoded as either UTF-8 or
UTF-16. The sqlite3_prepare() and sqlite3_prepare_v2() interfaces use UTF-8, and
sqlite3_prepare16() and sqlite3_prepare16_v2() use UTF-16. If the nByte argument
is less than zero, then zSql is read up to the first zero terminator. If nByte
        is non-negative, then it is the maximum number of bytes read from zSql.
When nByte is non-negative, the zSql string ends at either the first '\000' or
'\u0000' character or the nByte-th byte, whichever comes first. If the caller
knows that the supplied string is nul-terminated, then there is a small
performance advantage to be gained by passing an nByte parameter that is equal
to the number of bytes in the input string including the nul-terminator bytes.
If pzTail is not NULL then *pzTail is made to point to the first byte past the
end of the first SQL statement in zSql. These routines only compile the first
statement in zSql, so *pzTail is left pointing to what remains uncompiled.
*ppStmt is left pointing to a compiled prepared statement that can be executed
using sqlite3_step(). If there is an error, *ppStmt is set to NULL. If the input
text contains no SQL (if the input is an empty string or a comment) then *ppStmt
is set to NULL. The calling procedure is responsible for deleting the compiled
SQL statement using sqlite3_finalize() after it has finished with it. ppStmt may
not be NULL. On success, the sqlite3_prepare() family of routines return
SQLITE_OK; otherwise an error code is returned. The sqlite3_prepare_v2() and
sqlite3_prepare16_v2() interfaces are recommended for all new programs. The two
older interfaces are retained for backwards compatibility, but their use is
discouraged. In the "v2" interfaces, the prepared statement that is returned
(the sqlite3_stmt object) contains a copy of the original SQL text. This causes
the sqlite3_step() interface to behave differently in three ways:

        1.	If the database schema changes, instead of returning
SQLITE_SCHEMA as it always used to do, sqlite3_step() will automatically
recompile the SQL statement and try to run it again. If the schema has changed
in a way that makes the statement no longer valid, sqlite3_step() will still
return SQLITE_SCHEMA. But unlike the legacy behavior, SQLITE_SCHEMA is now a
                fatal error. Calling sqlite3_prepare_v2() again will not make
the error go away. Note: use sqlite3_errmsg() to find the text of the parsing
error that results in an SQLITE_SCHEMA return. 2.	When an error occurs,
sqlite3_step() will return one of the detailed error codes or extended error
codes. The legacy behavior was that sqlite3_step() would only return a generic
                SQLITE_ERROR result code and the application would have to make
a second call to sqlite3_reset() in order to find the underlying cause of the
problem. With the "v2" prepare interfaces, the underlying reason for the error
is returned immediately. 5.	If the value of a host parameter in the WHERE
clause might change the query plan for a statement, then the statement may be
automatically recompiled (as if there had been a schema change) on the first
sqlite3_step() call following any change to the bindings of the parameter.


int sqlite3_step(sqlite3_stmt*);

After a prepared statement has been prepared using either sqlite3_prepare_v2()
or sqlite3_prepare16_v2() or one of the legacy interfaces sqlite3_prepare() or
sqlite3_prepare16(), this function must be called one or more times to evaluate
the statement. The details of the behavior of the sqlite3_step() interface
depend on whether the statement was prepared using the newer "v2" interface
sqlite3_prepare_v2() and sqlite3_prepare16_v2() or the older legacy interface
sqlite3_prepare() and sqlite3_prepare16(). The use of the new "v2" interface is
recommended for new applications but the legacy interface will continue to be
        supported.
In the legacy interface, the return value will be either SQLITE_BUSY,
SQLITE_DONE, SQLITE_ROW, SQLITE_ERROR, or SQLITE_MISUSE. With the "v2"
interface, any of the other result codes or extended result codes might be
returned as well. SQLITE_BUSY means that the database engine was unable to
acquire the database locks it needs to do its job. If the statement is a COMMIT
or occurs outside of an explicit transaction, then you can retry the statement.
If the statement is not a COMMIT and occurs within a explicit transaction then
you should rollback the transaction before continuing. SQLITE_DONE means that
the statement has finished executing successfully. sqlite3_step() should not be
called again on this virtual machine without first calling sqlite3_reset() to
reset the virtual machine back to its initial state. If the SQL statement being
executed returns any data, then SQLITE_ROW is returned each time a new row of
data is ready for processing by the caller. The values may be accessed using the
column access functions. sqlite3_step() is called again to retrieve the next row
of data. SQLITE_ERROR means that a run-time error (such as a constraint
violation) has occurred. sqlite3_step() should not be called again on the VM.
More information may be found by calling sqlite3_errmsg(). With the legacy
interface, a more specific error code (for example, SQLITE_INTERRUPT,
SQLITE_SCHEMA, SQLITE_CORRUPT, and so forth) can be obtained by calling
        sqlite3_reset() on the prepared statement. In the "v2" interface, the
more specific error code is returned directly by sqlite3_step(). SQLITE_MISUSE
means that the this routine was called inappropriately. Perhaps it was called on
a prepared statement that has already been finalized or on one that had
previously returned SQLITE_ERROR or SQLITE_DONE. Or it could be the case that
the same database connection is being used by two or more threads at the same
moment in time. Goofy Interface Alert: In the legacy interface, the
sqlite3_step() API always returns a generic error code, SQLITE_ERROR, following
any error other than SQLITE_BUSY and SQLITE_MISUSE. You must call
sqlite3_reset() or sqlite3_finalize() in order to find one of the specific error
codes that better describes the error. We admit that this is a goofy design. The
problem has been fixed with the "v2" interface. If you prepare all of your SQL
statements using either sqlite3_prepare_v2() or sqlite3_prepare16_v2() instead
of the legacy sqlite3_prepare() and sqlite3_prepare16() interfaces, then the
more specific error codes are returned directly by sqlite3_step(). The use of
the "v2" interface is recommended.



int sqlite3_reset(sqlite3_stmt *pStmt);

The sqlite3_reset() function is called to reset a prepared statement object back
to its initial state, ready to be re-executed. Any SQL statement variables that
had values bound to them using the sqlite3_bind_*() API retain their values. Use
sqlite3_clear_bindings() to reset the bindings. The sqlite3_reset(S) interface
resets the prepared statement S back to the beginning of its program. If the
most recent call to sqlite3_step(S) for the prepared statement S returned
SQLITE_ROW or SQLITE_DONE, or if sqlite3_step(S) has never before been called on
S, then sqlite3_reset(S) returns SQLITE_OK. If the most recent call to
sqlite3_step(S) for the prepared statement S indicated an error, then
        sqlite3_reset(S) returns an appropriate error code.
The sqlite3_reset(S) interface does not change the values of any bindings on the
prepared statement S.



int sqlite3_finalize(sqlite3_stmt *pStmt);

The sqlite3_finalize() function is called to delete a prepared statement. If the
statement was executed successfully or not executed at all, then SQLITE_OK is
returned. If execution of the statement failed then an error code or extended
error code is returned. This routine can be called at any point during the
execution of the prepared statement. If the virtual machine has not completed
execution when this routine is called, that is like encountering an error or an
interrupt. Incomplete updates may be rolled back and transactions canceled,
depending on the circumstances, and the error code returned will be
SQLITE_ABORT.



int sqlite3_bind_blob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
SQLITE_TRANSIENT


*/
