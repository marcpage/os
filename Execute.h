#include <string>
#include <stdio.h>
#include "Exception.h"
#include "POSIXErrno.h"

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

namespace exec {
	/**
		@todo Document
		@todo Test
	*/
	inline std::string &execute(const std::string &command, std::string &stdout, const size_t blocks= 4096) {trace_scope
		FILE	*out= ::popen(command.c_str(), "r");

		ErrnoOnNULL(out);
		try {
			stdout.clear();
			while(!::feof(out)) {
				std::string::size_type	offset= stdout.size();
				off_t					amount;

				stdout.append(blocks, '\0');
				amount= ::fread(const_cast<char*>(&stdout.data()[offset]), 1, blocks, out);
				if(static_cast<size_t>(amount) != blocks) {
					stdout.erase(offset + amount);
					ErrnoCodeThrow(ferror(out), "stdout error");
				}
			}
		} catch(const std::exception&) {trace_scope
			::pclose(out);
			throw;
		}
		::pclose(out);
		return stdout;
	}
}
