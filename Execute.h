#include "Exception.h"
#include "POSIXErrno.h"
#include <stdio.h>
#include <string>

namespace exec {
/**
        @todo Document
*/
inline std::string &execute(const std::string &command, std::string &stdout,
                            const size_t blocks = 4096) {
  FILE *out = ::popen(command.c_str(), "r");

  ErrnoOnNULL(out);
  try {
    stdout.clear();
    while (!::feof(out)) {
      std::string::size_type offset = stdout.size();
      off_t amount;

      stdout.append(blocks, '\0');
      amount =
          ::fread(const_cast<char *>(&stdout.data()[offset]), 1, blocks, out);
      if (static_cast<size_t>(amount) != blocks) {
        stdout.erase(offset + amount);
        ErrnoCodeThrow(ferror(out), "stdout error");
      }
    }
  } catch (const std::exception &) {
    ::pclose(out); // not tested
    throw;         // not tested
  }
  ::pclose(out);
  return stdout;
}

inline std::string execute(const std::string &command,
                           const size_t blocks = 4096) {
  std::string output;

  return execute(command, output, blocks);
}
} // namespace exec
