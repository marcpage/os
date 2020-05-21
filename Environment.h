#ifndef __Environment_h__
#define __Environment_h__

/** @file Environment.h
        @todo document
*/

#include "POSIXErrno.h"
#include <map>
#include <string>
#include <unistd.h>

#if !defined(_GNU_SOURCE)
extern const char **environ;
#endif

namespace env {

enum Action { Overwrite, DoNotOverwrite };
typedef std::string String;
typedef std::map<String, String> Dictionary;

bool has(const String &name) { return NULL != ::getenv(name.c_str()); }
String get(const String &name) {
  const char *value = ::getenv(name.c_str());

  if (NULL == value) {
    return "";
  }
  return value;
}
void set(const String &name, const String &value, Action action = Overwrite) {
  ErrnoOnNegative(
      ::setenv(name.c_str(), value.c_str(), Overwrite == action ? 1 : 0));
}
void clear(const String &name) { ErrnoOnNegative(::unsetenv(name.c_str())); }
Dictionary &list(Dictionary &env) {

  env.clear();
  for (int i = 0; NULL != environ[i]; ++i) {
    String field = environ[i];
    String::size_type equalPos = field.find('=');

    if (String::npos == equalPos) {
      env[field] = ""; // not tested
    } else {
      env[field.substr(0, equalPos)] = field.substr(equalPos + 1);
    }
  }
  return env;
}

} // namespace env

#endif // __Environment_h__
