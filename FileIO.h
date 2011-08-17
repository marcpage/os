#ifndef __FileIO_h__
#define __FileIO_h__

#include "Exception.h"
#include <errno.h>

namespace io {
	inline std::string &load(const std::string &path, std::string &contents) {
		FILE	*file= fopen(path.c_str(), "rb");
		off_t	size;

		errnoNULLAssertMessageException(file);
		try {
			errnoAssertPossitiveMessageException(fseeko(file, 0, SEEK_END));
			errnoAssertPossitiveMessageException(size= ftello(file));
			rewind(file);
			contents.assign(size, '\0');
			size= fread(const_cast<char*>(contents.data()), 1, static_cast<size_t>(size), file);
			AssertMessageException(ferror(file) == 0);
			contents.resize(size);
		} catch (const std::exception&) {
			fclose(file);
			throw;
		}
		return contents;
	}

	inline void save(const std::string &path, const std::string &contents) {
		FILE	*file= fopen(path.c_str(), "wb");
		off_t	written;

		errnoNULLAssertMessageException(file);
		try {
			written= fwrite(contents.data(), 1, static_cast<size_t>(contents.size()), file);
			AssertMessageException(ferror(file) == 0);
		} catch (const std::exception&) {
			fclose(file);
			throw;
		}
	}

}

#endif // __FileIO_h__
