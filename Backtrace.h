#ifndef __Backtrace_h__
#define __Backtrace_h__

/** @file Backtrace.h
	@todo document
	@todo Unmangle using abi::__cxa_demangle
*/

#include <string>
#include <vector>
#include <execinfo.h>
#include <stdlib.h>

namespace trace {

	typedef std::string String;
	typedef std::vector<String> StringList;

	inline StringList &stack(StringList &list, int maxDepth=4096) {
		void	**frames= new void*[maxDepth];
		int		count= ::backtrace(frames, maxDepth);
		char	**names= ::backtrace_symbols(frames, count);

		list.clear();
		for (int i= 0; i < count; ++i) {
			list.push_back(names[i]);
		}
		::free(names);
		delete[] frames;
		return list;
	}
	inline StringList stack(int maxDepth=4096) {
		StringList	list;

		return stack(list, maxDepth);
	}
	inline void print(int maxDepth=4096) {
		StringList names= stack(maxDepth);

		for(StringList::iterator i= names.begin(); i != names.end(); ++i) {
			printf("%s\n", i->c_str());
		}
	}
}

#endif // __Backtrace_h__
