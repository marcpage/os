#ifndef __Library_h__
#define __Library_h__

/** @file Library.h
	Dynamically Loaded Shared Library API.
	Supports dlopen, CFBundle and HMODULE on the appropriate platforms.
	@todo Evaluate Windows exception throwing Macros and see if they compile.
			You can even set it to #if !defined(__FUNCTION__) to see if that works.
	@todo See if __FUNCTION__ or __func__ is the appropriate macro
*/

// --- C++ Headers ---

#include <exception>
#include <string>

// --- System Headers ---

#if defined(__APPLE__)	// http://predef.sourceforge.net/preos.html#sec20
	#include <CoreFoundation/CoreFoundation.h>
	#include <Carbon/Carbon.h>
	#define __use_bundles__	1 ///< CFBundle API supported
	#define __use_something__ ///< We've figured out a library API
#endif
#if defined(__APPLE__) || defined(linux) // http://predef.sourceforge.net/preos.html#sec18
	#include <dlfcn.h>
	#define __use_dlopen__	1				///< dlopen API supported
	#define __use_something__				///< We've figured out a library API
	#define __std_lib_prefix__	"lib"		///< dlopen libraries usually start with lib
	#if  defined(__APPLE__)
		#define __std_lib_suffix__	".dylib" ///< dlopen libraries on Mac are usually dylib
	#else
		#define __std_lib_suffix__	".so"	///< dlopen libraries on linux are .so
	#endif
	#define __path_separator__	'/'			///< UNIX platforms use the / separator
#endif
#if defined(_WIN32) // http://predef.sourceforge.net/preos.html#sec25
	#include <windows.h>
	#define __use_module__	1			///< HMODULE API supported
	#define __use_something__			///< We've figured out a library API
	#define __std_lib_prefix__	NULL	///< No prefix for libraries on Windows
	#define __std_lib_suffix__	".dll"	///< dll is the standard extension
	#define __path_separator__	'\\'	///< \ is the DOS path separator
#endif
#if !defined(__use_something__)
	#error Need to port to this platform
#endif


// --- Library Class ---

namespace sys {

	/** An abstraction of a system library.
	*/
	class Library {
		public:
			/// Library at a given path
			Library(const char *path);
			/// Close the library
			~Library();
			/// Lookup a function in the library
			template<class Function> Function function(const char *name);

			/** Exceptions thrown from this library
			*/
			class Exception : public std::exception {
				public:
					/// Exception to throw if compiler does not support function
					Exception(const std::string &message, const char *file, int line) throw();
					/// Exception to throw if compiler does support function
					Exception(const std::string &message, const char *file, int line, const char *function) throw();
					/// cleanup std::string
					virtual ~Exception() throw();
					/// get the file, line (maybe function) and message of the exception
					virtual const char* what() const throw();
				private:
					std::string	_message;	///< The fully constructed message
					/// common code for constructors
					static std::string _buildMessage(const std::string &message, const char *file, int line, const char *function= NULL);
			};

		private:
			#if __use_dlopen__
				void		*_dl;			///< dlopen handle
			#endif
			#if __use_bundles__
				CFBundleRef	_bundle;		///< CoreFoundation bundle
			#endif
			#if __use_module__
				HINSTANCE	*_module;		///< Windows module
			#endif
			/// Determine if we've mucked with the name passed to us
			enum PathModified {
				Modified,	// the path passed in is not modified
				Unmodified	// the path passed in has a suffix, prefix of both added
			};
			#if __use_bundles__
				/// Loads the named bundle and the given location
				bool _load(CFURLRef base, CFStringRef name);
			#endif
			/// Searches for a bundle by the given name
			bool _search_bundle(const std::string &name);
			/// Searches for the requested path
			bool _attempt_core(const char *path, PathModified modified);
			Library();	///< Prevent Usage
			Library(const Library&); ///< Prevent Usage
			Library &operator=(const Library&); ///< Prevent Usage
	};
}


// --- Exception Macros for implementation ---

#if defined(__FUNCTION__)
	/// Throw an exception if condition isn't met
	#define sysLibraryAssert(condition,message) if(!(condition)) {throw sys::Library::Exception(std::string(#condition)+(message), __FILE__, __LINE__, __FUNCTION__);} else
	/// Throw an exception if the expression is NULL
	#define sysLibraryAssertNotNULL(variable,message) if(NULL==(variable)) {throw sys::Library::Exception(std::string(#variable " was NULL:")+message, __FILE__, __LINE__, __FUNCTION__);} else
#else
	/// Throw an exception if condition isn't met
	#define sysLibraryAssert(condition,message) if(!(condition)) {throw sys::Library::Exception(std::string("("#condition") failed:")+(message), __FILE__, __LINE__);} else
	/// Throw an exception if the expression is NULL
	#define sysLibraryAssertNotNULL(variable,message) if(NULL==(variable)) {throw sys::Library::Exception(std::string(#variable " was NULL:")+message, __FILE__, __LINE__);} else
#endif


// --- Implementation ---

namespace sys {


	// --- Library Implementation ---

	/** Gets the library at the given path.
		@param path	The path to the library. <b>Mac OS X</b>: Can also be a bundle identifier.
		@throw Library::Exception	on error
	*/
	inline Library::Library(const char *path):
		#if __use_dlopen__
			_dl(NULL)
		#endif
		#if __use_bundles__
			, _bundle(NULL)
		#endif
		#if __use_module__
			_module(NULL)
		#endif
	{
		bool				found= _attempt_core(path, Unmodified); // try a quick and simple load of what they gave us
		const bool			justAName= (NULL == strchr(path, __path_separator__));
		const std::string	str(path);

		if(!found && justAName ) {

			if(NULL != __std_lib_prefix__) { // try adding a prefix
				found= _attempt_core((__std_lib_prefix__+str).c_str(), Modified);
			}
			if(!found && (NULL != __std_lib_suffix__) ) { // try adding a suffix
				found= _attempt_core((str+__std_lib_suffix__).c_str(), Modified);
			}
			if(!found && (NULL != __std_lib_prefix__) && (NULL != __std_lib_suffix__) ) { // try adding both
				found= _attempt_core((__std_lib_prefix__+str+__std_lib_suffix__).c_str(), Modified);
			}
		}
		#if __use_bundles__
			if(!found) { // try the different bundle extensions
				static const char * const bundleSuffixes[]= {
					"", ".framework", ".bundle", ".plugin", ".app", ".kext"
				};
				for(size_t index= 0; !found && index < sizeof(bundleSuffixes)/sizeof(bundleSuffixes[0]); ++index) {
					if(justAName) {
						found= _search_bundle(str+bundleSuffixes[index]);
					} else {
						found= _attempt_core((str+bundleSuffixes[index]).c_str(), Unmodified);
						// technically we're lying about Unmodified above, but it is the intended behavior we want
					}
				}
			}
		#endif
		#if __use_dlopen__
			if(!found) { // get dlerror() loaded with the message for the original path error
				found= _attempt_core(path, Unmodified);
			}
		#endif
		sysLibraryAssert(found, std::string("Unable to open library: ")+path);
	}
	/** Close the library.
	*/
	inline Library::~Library() {
		#if __use_module__
			if(NULL != _module) {
				FreeLibrary(_module);
			}
		#endif
		#if __use_bundles__
			if(NULL != _bundle) {
				CFRelease(_bundle);
				_bundle= NULL;
			}
		#endif
		#if __use_dlopen__
			if(NULL != _dl) {
				sysLibraryAssert(0 == dlclose(_dl), "Unable to close library");
				_dl= NULL;
			}
		#endif
	}
	/** Gets a function pointer from the library.
		@tparam Function	The typedef for the function expected.
		@param name			The name of the function to lookup.
		@throw Exception	If the function does not exist, or any other problems.
	*/
	template<class Function> inline Function Library::function(const char *name) {
		void	*ptr= NULL;

		#if __use_module__
			if(NULL != _module) {
				ptr= GetProcAddress(_moudle, reinterpret_cast<LPSTR>(name));
			}
		#endif
		#if __use_bundles__
			if(NULL != _bundle) {
				CFStringRef		str= CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(name), strlen(name), CFStringGetSystemEncoding(), false);

				ptr= (NULL != str) ? CFBundleGetFunctionPointerForName(_bundle, str) : NULL;
				if(NULL != str) {
					CFRelease(str);
				}
			}
		#endif
		#if __use_dlopen__
			if(NULL != _dl) {
				ptr= dlsym(_dl, name);
			}
		#endif
		sysLibraryAssertNotNULL(ptr, std::string("Function not found: ")+name);
		return reinterpret_cast<Function>(ptr);
	}
	/** Loads a bundle with the given filename from the given directory.
		@param base	The directory to load from. <b>NOTE</b>: CFRelease <b>will</b> be called on this.
		@param name	The name of the bundle directory to potentially load.
						 <b>NOTE</b>: CFRelease <b>will not</b> be called on this.
		@return		true if we were able to load the bundle successfully.
	*/
#if __use_bundles__
	inline bool Library::_load(CFURLRef base, CFStringRef name) {
		CFURLRef	itemPath= (NULL != base) && (NULL != name)
						? CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, base, name, true) : NULL;

		//CFShow(itemPath ? (CFTypeRef)itemPath : (CFTypeRef)CFSTR("NULL"));
		_bundle= (NULL != itemPath) ? CFBundleCreate(kCFAllocatorDefault, itemPath) : NULL;
		if(NULL != base) {
			CFRelease(base);
		}
		if(NULL != itemPath) {
			CFRelease(itemPath);
		}
		return NULL != _bundle;
	}
#endif // __use_bundles__
	/** Searches for a bundle with the given name in the standard locations.
			Searches in the application bundle locations and user/local/system locations for bundles.
			See constants at the beginning of the function to see all the places it searches.
			<b>NOTE</b>: Does nothing unless __use_bundles__
		@param name	The name of the bundle to load (including extension).
		@return		true if we found a bundle by that name
	*/
	inline bool Library::_search_bundle(const std::string &name) {
		#if __use_bundles__
			typedef CFURLRef (*BundlePath)(CFBundleRef);
			static const FSVolumeRefNum	domains[]= {kUserDomain, kLocalDomain, kSystemDomain};
			static const OSType			folders[]= {
				kExtensionFolderType,			kApplicationSupportFolderType,	kInternetPlugInFolderType,
				kSharedLibrariesFolderType,		kContextualMenuItemsFolderType,	kQuickTimeExtensionsFolderType,
				kDisplayExtensionsFolderType,	kPrintingPlugInsFolderType,
			};
			static const BundlePath	paths[]= {
				CFBundleCopyPrivateFrameworksURL,	CFBundleCopySharedFrameworksURL,
				CFBundleCopyBuiltInPlugInsURL,		CFBundleCopyResourcesDirectoryURL,
				CFBundleCopySharedSupportURL,		CFBundleCopySupportFilesDirectoryURL,
				CFBundleCopyBundleURL,
			};
			static const char * const absolutePaths[]= {
				"/Library/Frameworks", "/System/Library/Frameworks", "/System//Library/PrivateFrameworks"
			};
			CFBundleRef	mainBundle= CFBundleGetMainBundle();
			CFStringRef	str= CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(name.data()), name.length(), CFStringGetSystemEncoding(), false);

			// Search inside the main bundle, if there is one
			if( (NULL != mainBundle) && (NULL != str) ) {
				for(size_t index= 0; index < sizeof(paths)/sizeof(paths[0]); ++index) {
					if(false) {// if(_load(paths[index](mainBundle), str)) {
						return true;
					}
				}
			}
			// Search in the standard folders
			for(size_t domain= 0; str && (domain < sizeof(domains)/sizeof(domains[0])); ++domain) {
				for(size_t folder= 0; folder < sizeof(folders)/sizeof(folders[0]); ++folder) {
					FSRef	systemFolder;

					if(noErr == FSFindFolder(domains[domain], folders[folder], false /*create*/, &systemFolder)) {
						if(_load(CFURLCreateFromFSRef(kCFAllocatorDefault, &systemFolder), str)) {
							return true;
						}
					}
				}
			}
			// Search in some hard coded, last ditch locations
			for(size_t path= 0; str && (path < sizeof(absolutePaths)/sizeof(absolutePaths[0])); ++path) {
				if(_load(CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(absolutePaths[path]), strlen(absolutePaths[path]), true), str)) {
					return true;
				}
			}
			if(NULL != str) {
				CFRelease(str);
			}
		#endif
		return false;
	}
	/** Attempts to load a library with the given path/name/identifier.
		@param path		The path, name or identifier to load.
		@param modified	Is path the one passed in from the client?
		@return			true if we were able to load the library.
	*/
	inline bool Library::_attempt_core(const char *path, PathModified modified) {
		#if __use_module__
			_module= LoadLibrary(reinterpret_cast<LPCSTR>(path));
			if(NULL != _module) {
				return true;
			}
		#endif
		#if __use_bundles__
			if(Unmodified == modified) {
				const size_t	len= strlen(path);
				CFStringRef		str;
				CFURLRef		url= CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(path), len, false);

				_bundle= (NULL != url) ? CFBundleCreate(kCFAllocatorDefault, url) : NULL;

				 // if not a path, try it as a bundle identifier
				str= (NULL != _bundle) ? NULL : CFStringCreateWithBytes(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(path), len, CFStringGetSystemEncoding(), false);
				_bundle= (NULL != str) ? CFBundleGetBundleWithIdentifier(str) : _bundle;
				if(NULL != str) {
					CFRelease(str);
				}
				if(NULL != _bundle) {
					return true;
				}
			}
		#endif
		#if __use_dlopen__
			_dl= dlopen(path, RTLD_NOW);
			if(NULL != _dl) {
				return true;
			}
		#endif
		return false;
	}


	// --- Library Exception Implementation ---

	/** Creates a new library exception.
		@param message	The message about what was going on when the exception was thrown.
		@param file		pass __FILE__
		@param line		pass __LINE__
	*/
	inline Library::Exception::Exception(const std::string &message, const char *file, int line) throw()
		:_message(_buildMessage(message, file, line)) {}
	/** Creates a new library exception for when you know what function you are in.
		@param message	The message about what was going on when the exception was thrown.
		@param file		pass __FILE__
		@param line		pass __LINE__
		@param function	pass __FUNCTION__
	*/
	inline Library::Exception::Exception(const std::string &message, const char *file, int line, const char *function) throw()
		:_message(_buildMessage(message, file, line, function)) {}
	/** Cleans up the _message string.
	*/
	inline Library::Exception::~Exception() throw() {}
	/** Gets the _message string.
	*/
	inline const char *Library::Exception::what() const throw() {
		return _message.c_str();
	}
	/** Builds up a message about the exception.
		Message will be of the format:
			(file):(line):[function:][dlerror:](message)
		<b>Mac OS X and Linux</b>: If there is a dlerror(), it will be used also.
		@param message	The user message about what was going on.
		@param file		__FILE__
		@param line		__LINE__
		@param function	__FUNCTION__ or NULL
	*/
	inline std::string Library::Exception::_buildMessage(const std::string &message, const char *file, int line, const char *function) {
		std::string	result(file);

		result.append(1, ':');
		if(line < 0) {
			line*= -1;
			result.append(1, '-');
		}
		if(0 == line) {
			result.append(1, '0');
		} else {
			const std::string::size_type	position= result.length();

			while(line > 0) {
				result.insert(position, 1, '0'+(line%10));
				line/= 10;
			}
		}
		result.append(1, ':');
		if(NULL != function) {
			result.append(function);
			result.append(1, ':');
		}
		#if __use_dlopen__
			const char *dlerrorMsg= dlerror();
			if(NULL != dlerrorMsg) {
				result.append(dlerrorMsg);
				result.append(1, ':');
			}
		#endif
		return result+message;
	}
}


// --- Cleanup ---

#undef sysLibraryAssert			// don't export
#undef sysLibraryAssertNotNULL	// don't export
#undef __use_bundles__			// don't export
#undef __use_dlopen__			// don't export
#undef __use_module__			// don't export
#undef __use_something__		// don't export
#undef __std_lib_prefix__		// don't export
#undef __std_lib_suffix__		// don't export
#undef __path_separator__		// don't export

#endif // __Library_h__
