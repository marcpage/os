#ifndef __SystemIdentification_h__
#define __SystemIdentification_h__

#include <string>
#include <sys/utsname.h>
#include "os/POSIXErrno.h"

namespace sys {
	/*

	See:
	https://stackoverflow.com/questions/11072804/how-do-i-determine-the-os-version-at-runtime-in-os-x-or-ios-without-using-gesta

	sysname: Darwin
	nodename: crazyhorse.amer.corp.natinst.com
	release: 18.7.0
	version: Darwin Kernel Version 18.7.0: Sat Oct 12 00:02:19 PDT 2019; root:xnu-4903.278.12~1/RELEASE_X86_64
	machine: x86_64
	*/
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
