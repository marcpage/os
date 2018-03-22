#ifndef __Environment_h__
#define __Environment_h__

/** @file Environment.h
	@todo document
*/

#include <string>
#include "POSIXErrno.h"

namespace env {

	enum Action {
		Overwrite,
		DoNotOverwrite
	};
	bool has(const std::string &name) {
		return NULL != ::getenv(name.c_str());
	}
	std::string get(const std::string &name) {
		const char *value = ::getenv(name.c_str());

		if (NULL == value) {
			return "";
		}
		return value;
	}
	void set(const std::string &name, const std::string &value, Action action=Overwrite) {
		ErrnoOnNegative(::setenv(name.c_str(), value.c_str(), Overwrite == action ? 1 : 0));
	}
	void clear(const std::string &name) {
		ErrnoOnNegative(::unsetenv(name.c_str()));
	}

}

#endif // __Environment_h__
