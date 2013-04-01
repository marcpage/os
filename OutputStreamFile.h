#ifndef __OutputStreamFile_h__
#define __OutputStreamFile_h__

#include "OutputStream.h"
#include "Exception.h"
#include <string>

namespace io {

	class OutputStreamFile : public OutputStream {
	public:
		OutputStreamFile(FILE *file);
		virtual ~OutputStreamFile();
	protected:
		virtual void writeBytes(const void *buffer, int count);
		virtual void flushBytes();
		virtual void closeStream();
	private:
		FILE	*_file;
	};

	inline OutputStreamFile::OutputStreamFile(FILE *file)
			:_file(file) {}
	inline OutputStreamFile::~OutputStreamFile() {}
	inline void OutputStreamFile::writeBytes(const void *buffer, int count) {
		off_t	amount;

		AssertMessageException(count == fwrite(reinterpret_cast<const char*>(buffer), 1, count, _file));
		AssertMessageException(ferror(_file) == 0);
	}
	inline void OutputStreamFile::flushBytes() {
		ErrnoOnNegative(fflush(_file));
	}
	inline void OutputStreamFile::closeStream() {
		if(NULL != _file) {
			fclose(_file);
			_file= NULL;
		}
	}
}

#endif // __OutputStreamFile_h__
