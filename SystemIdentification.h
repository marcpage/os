#ifndef __SystemIdentification_h__
#define __SystemIdentification_h__

#include <string>
#include <sys/utsname.h>
#include "os/POSIXErrno.h"

namespace sys {
	inline void info(std::string &sysname, std::string &nodename, std::string &release, std::string &version, std::string &machine) {
		struct utsname systemInfo;

		ErrnoOnNegative(::uname(&systemInfo));
		sysname = systemInfo.sysname;
		nodename = systemInfo.nodename;
		release = systemInfo.release;
		version = systemInfo.version;
		machine = systemInfo.machine;
	}
	inline std::string osName() {
		std::string sysname, nodename, release, version, machine;

		info(sysname, nodename, release, version, machine);
		return sysname;
	}
	inline std::string nodeName() {
		std::string sysname, nodename, release, version, machine;

		info(sysname, nodename, release, version, machine);
		return nodename;
	}
	inline std::string osRelease() {
		std::string sysname, nodename, release, version, machine;

		info(sysname, nodename, release, version, machine);
		return release;
	}
	inline std::string systemVersion() {
		std::string sysname, nodename, release, version, machine;

		info(sysname, nodename, release, version, machine);
		return version;
	}
	inline std::string architecture() {
		std::string sysname, nodename, release, version, machine;

		info(sysname, nodename, release, version, machine);
		return machine;
	}
}

#endif // __SystemIdentification_h__

/*
     int
     uname(struct utsname *name);

DESCRIPTION
     The uname() function stores nul-terminated strings of information identifying the current system into
     the structure referenced by name.

     The utsname structure is defined in the <sys/utsname.h> header file, and contains the following mem-
     bers:

           sysname       Name of the operating system implementation.

           nodename      Network name of this machine.

           release       Release level of the operating system.

           version       Version level of the operating system.

           machine       Machine hardware platform.

     The uname() function returns the value 0 if successful; otherwise the value -1 is returned and the
     global variable errno is set to indicate the error.
*/
