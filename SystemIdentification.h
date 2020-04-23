#ifndef __SystemIdentification_h__
#define __SystemIdentification_h__

#include "os/POSIXErrno.h"
#include <string>
#include <sys/utsname.h>

namespace sys {

/** Get information about this machine.
        See:
        https://stackoverflow.com/questions/11072804/how-do-i-determine-the-os-version-at-runtime-in-os-x-or-ios-without-using-gesta

        sysname: Darwin
        nodename: crazyhorse.amer.corp.natinst.com
        release: 18.7.0
        version: Darwin Kernel Version 18.7.0: Sat Oct 12 00:02:19 PDT 2019;
        root:xnu-4903.278.12~1/RELEASE_X86_64
        machine: x86_64
        @param sysname The name of the type of operating system. macOS => Darwin
        @param nodename The fully qualified domain name.
        @param release The version of sysname. macOS 10.15.3 => 19.3.0
        @param version All the information about the version of sysname.
        @param machine The type of processor. 64-bit Intel => x86_64
*/
inline void info(std::string &sysname, std::string &nodename,
                 std::string &release, std::string &version,
                 std::string &machine) {
  struct utsname systemInfo;

  ErrnoOnNegative(::uname(&systemInfo));
  sysname = systemInfo.sysname;
  nodename = systemInfo.nodename;
  release = systemInfo.release;
  version = systemInfo.version;
  machine = systemInfo.machine;
}
/** The name of the type of operating system.
        macOS => Darwin
        @return THe name of the operating system.
*/
inline std::string osName() {
  std::string sysname, nodename, release, version, machine;

  info(sysname, nodename, release, version, machine);
  return sysname;
}
/** The name of the machine.
        @return The fully qualified domain name.
*/
inline std::string nodeName() {
  std::string sysname, nodename, release, version, machine;

  info(sysname, nodename, release, version, machine);
  return nodename;
}
/** The operating system version.
        macOS 10.15.3 => 19.3.0
*/
inline std::string osRelease() {
  std::string sysname, nodename, release, version, machine;

  info(sysname, nodename, release, version, machine);
  return release;
}
/** The full operating system version.
        For an example of the type of information returned:
        Darwin Kernel Version 19.3.0: Thu Jan  9 20:58:23 PST 2020;
   root:xnu-6153.81.5~1/RELEASE_X86_64
        @return All the information about this version of the operating system.
*/
inline std::string systemVersion() {
  std::string sysname, nodename, release, version, machine;

  info(sysname, nodename, release, version, machine);
  return version;
}
/** The type of processor architecture.
        For instance x86_64.
        @return The processor type.
*/
inline std::string architecture() {
  std::string sysname, nodename, release, version, machine;

  info(sysname, nodename, release, version, machine);
  return machine;
}

} // namespace sys

#endif // __SystemIdentification_h__
