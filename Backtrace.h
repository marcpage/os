#ifndef __Backtrace_h__
#define __Backtrace_h__

/** @file Backtrace.h
        @todo document
        @todo Unmangle using abi::__cxa_demangle
*/

#include <cxxabi.h> // abi::__cxa_demangle
#include <execinfo.h>
#include <stdlib.h>
#include <string>
#include <vector>

namespace trace {

typedef std::string String;
typedef std::vector<String> StringList;

/*

        0   Backtrace_clang++_trace             0x000000010c8783f0
   _ZN5trace5stackERNSt3__16vectorINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS5_IS7_EEEEi
   + 112
*/
inline std::string demangleName(const std::string &name) {
  size_t dataSize = 0;
  int status = 0;
  std::string result;

  const char *unmangled =
      abi::__cxa_demangle(name.c_str(), nullptr, &dataSize, &status);

  if (0 == status) {
    result = unmangled;
  } else {
    result = name;
  }
  ::free(reinterpret_cast<void *>(const_cast<char *>(unmangled)));
  return result;
}
inline std::string demangleLine(const std::string &line) {
  std::string::size_type start, end;
  end = line.rfind('+');
  if ((std::string::npos == end) || (end < 3) || (' ' != line[end - 1])) {
    return line;
  }

  end -= 1;
  start = line.rfind(' ', end - 1);
  if ((std::string::npos == end) || (start < 3) || (end - start < 1)) {
    return line;
  }

  ++start;
  return line.substr(0, start) + demangleName(line.substr(start, end - start)) +
         line.substr(end);
}
/*
        0   Backtrace_clang++_trace             0x000000010c8783f0
   _ZN5trace5stackERNSt3__16vectorINS0_12basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEENS5_IS7_EEEEi
   + 112
*/
inline std::string stripPrefix(const std::string &line) {
  std::string::size_type zeroEx = line.find("0x");

  if (std::string::npos != zeroEx) {
    return line.substr(zeroEx + 2);
  }
  return line;
}
inline StringList &stack(StringList &list, int maxDepth = 4096) {
  const int skipStackFrames =
      4; // trace::stack() call, std::string, and Exception x 2
  void **frames = new void *[maxDepth];
  int count = ::backtrace(frames, maxDepth);
  char **names = ::backtrace_symbols(frames, count);

  list.clear();
  for (int i = skipStackFrames; i < count; ++i) {
    list.push_back(demangleLine(stripPrefix(names[i])));
  }
  ::free(names);
  delete[] frames;
  return list;
}
inline StringList stack(int maxDepth = 4096) {
  StringList list;

  return stack(list, maxDepth);
}
inline void print(int maxDepth = 4096) {
  StringList names = stack(maxDepth);

  for (StringList::iterator i = names.begin(); i != names.end(); ++i) {
    printf("%s\n", i->c_str());
  }
}
} // namespace trace

#endif // __Backtrace_h__
