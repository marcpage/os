#ifndef __MemoryMappedFile_h__
#define __MemoryMappedFile_h__

#include "POSIXErrno.h"
#include "FileDescriptor.h"
#include <sys/mman.h> // mmap
#include <string>

namespace io {

	class MemoryMappedFile {
		public:
			MemoryMappedFile(const FileDescriptor &file, size_t size=0, size_t offset=0, int protections=PROT_READ|PROT_WRITE, int flags=MAP_SHARED);
			MemoryMappedFile(const std::string &file, size_t size=0, size_t offset=0, int protections=PROT_READ|PROT_WRITE, int flags=MAP_SHARED);
			operator void*();
			size_t size();
			template<class T>
			T *address();
			virtual ~MemoryMappedFile();
		private:
			size_t _size;
			void *_address;
			MemoryMappedFile(const MemoryMappedFile&); ///< Mark as unusable
			MemoryMappedFile &operator=(const MemoryMappedFile&); ///< Mark as unusable
	};

}

namespace io {
	/**
		@todo Test!
	*/
	inline MemoryMappedFile::MemoryMappedFile(const FileDescriptor &file, size_t size, size_t offset, int protections, int flags)
		:_size(size > 0 ? size : (file.size() - offset)),
		 _address(ErrnoOnNULL(::mmap(nullptr, _size, protections, flags, file, offset))) {}
	/**
		@todo Test!
	*/
	inline MemoryMappedFile::MemoryMappedFile(const std::string &file, size_t size, size_t offset, int protections, int flags)
				:_size(0), _address(nullptr) {
			FileDescriptor fd(file);

			_size = size > 0 ? size : (fd.size() - offset);
			_address = ErrnoOnNULL(::mmap(nullptr, _size, protections, flags, fd, offset));
		}
	/**
		@todo Test!
	*/
	inline MemoryMappedFile::operator void*() {
		return address<void>();
	}
	/**
		@todo Test!
	*/
	size_t MemoryMappedFile::size() {
		return _size;
	}
	/**
		@todo Test!
	*/
	template<class T>
	inline T *MemoryMappedFile::address() {
		return reinterpret_cast<T*>(_address);
	}

	inline MemoryMappedFile::~MemoryMappedFile() {
		try {
			ErrnoOnNegative(::munmap(_address, _size));
		} catch(const std::exception &) {
			// cannot handle error in exception
		}
	}

}

#endif // __MemoryMappedFile_h__
