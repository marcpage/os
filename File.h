#ifndef __File_h__
#define __File_h__

#include "Exception.h"
#include <string>

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

#if _DEBUG_FILE // Debug
inline FILE *printResult(FILE *t) {printf("\t RESULT: %08x\n", t); return t;}
inline off_t printResult(off_t t) {printf("\t RESULT: %d,%d\n", t); return t;}
#define fopen(p,m) (printf("fopen(%s, %s)\n", p, m), printResult(fopen(p,m)))
#define ftello(f) (printf("ftello(%08x)\n", f), printResult(ftello(f)))
#define fseeko(f,o,w) (printf("fseeko(%08x, %d, %d)\n", f, o, w), printResult(fseeko(f,o,w)))
#define fread(b,s,n,f) (printf("fread(%08x, %d, %d, %08x)\n", b, s, n, f), printResult(fread(b,s,n,f)))
#define fwrite(b,s,n,f) (printf("fwrite(%08x, %d, %d, %08x)\n", b, s, n, f), printResult(fwrite(b,s,n,f)))
#define fflush(f) (printf("fflush(%08x)\n", f), printResult(fflush(f)))
#define fclose(f) (printf("fclose(%08x)\n", f), printResult(fclose(f)))
#define ferror(f) (printf("ferror(%08x)\n", f), printResult(ferror(f)))
#endif

namespace io {

	class File {
		public:
			enum Method		{Binary, Text};
			enum Protection	{ReadOnly, ReadWrite, WriteIfPossible};
			enum Relative	{FromHere, FromStart, FromEnd};
			enum Endian		{BigEndian, LittleEndian, NativeEndian};
			File(const char *path, Method method, Protection protection);
			File(const std::string &path, Method method, Protection protection);
			virtual ~File();
			off_t size() const;
			void flush();
			off_t location() const;
			bool writable() const;
			void moveto(off_t offset, Relative relative= FromStart) const;
			void move(off_t offset, Relative relative= FromHere) const;
			void read(void *buffer, size_t bufferSize, off_t offset= 0, Relative relative= FromHere) const;
			void write(const void *buffer, size_t bufferSize, off_t offset= 0, Relative relative= FromHere);
			std::string &read(std::string &buffer, size_t bufferSize= static_cast<size_t>(-1), off_t offset= 0, Relative relative= FromHere) const;
			void write(const std::string &buffer, off_t offset= 0, Relative relative= FromHere);
			template<class Int> Int read(Endian endian, off_t offset= 0, Relative relative= FromHere) const;
			template<class Int> void write(Int number, Endian endian, off_t offset= 0, Relative relative= FromHere);
		private:
			FILE	*_file;
			bool	_readOnly;
			static int _whence(Relative relative);
			void _goto(off_t offset, Relative relative) const;
			static Endian _actualEndian(Endian endian);
			static FILE *_open(const char *path, Method method, Protection protection, bool &readOnly);
			File(const File&); ///< Mark as unusable
			File &operator=(const File&); ///< Mark as unusable
	};

}

#define	kReadOnlyText			"r"
#define	kOpenReadWriteText		"r+"
#define kCreateReadWriteText	"w+"
#define	kReadOnlyBinary			"rb"
#define	kOpenReadWriteBinary	"r+b"
#define kCreateReadWriteBinary	"w+b"

namespace io {

	/**
		@todo Test!
	*/
	inline File::File(const char *path, Method method, Protection protection)
		:_file(NULL), _readOnly(ReadOnly == protection) {trace_scope
		_file= _open(path, method, protection, _readOnly);
		moveto(0);
	}
	/**
		@todo Test!
	*/
	inline File::File(const std::string &path, Method method, Protection protection)
		:_file(NULL), _readOnly(ReadOnly == protection) {trace_scope
		_file= _open(path.c_str(), method, protection, _readOnly);
		moveto(0);
	}
	inline File::~File() {trace_scope
		if(NULL != _file) {
			fclose(_file);
			_file= NULL;
		}
	}
	/**
		@todo Test!
	*/
	inline off_t File::size() const {trace_scope
		off_t	here= location();
		off_t	end;

		moveto(0, FromEnd);
		end= location();
		moveto(here, FromStart);
		return end;
	}
	/**
		@todo Test!
	*/
	inline void File::flush() {trace_scope
		errnoAssertPositiveMessageException(fflush(_file));
	}
	/**
		@todo Test!
	*/
	inline off_t File::location() const {trace_scope
		off_t	currentPos;

		errnoAssertPositiveMessageException(currentPos= ftello(_file));
		return currentPos;
	}
	/**
		@todo Test!
	*/
	inline bool File::writable() const {trace_scope
		return trace_bool(!_readOnly);
	}
	inline void File::moveto(off_t offset, Relative relative) const {trace_scope
		errnoAssertPositiveMessageException(fseeko(_file, offset, _whence(relative)));
	}
	/**
		@todo Test!
	*/
	inline void File::move(off_t offset, Relative relative) const {trace_scope
		move(offset, relative);
	}
	/**
		@todo Test!
	*/
	inline void File::read(void *buffer, size_t bufferSize, off_t offset, Relative relative) const {trace_scope
		off_t	amount;
		int		fileError;

		_goto(offset, relative);
		AssertMessageException(location() + static_cast<off_t>(bufferSize) <= size());
		amount= fread(reinterpret_cast<char*>(buffer), 1, bufferSize, _file);
		AssertMessageException( ((fileError= ferror(_file)) == 0) || (fileError == EOF) );
		AssertMessageException(amount == static_cast<off_t>(bufferSize));
	}
	inline void File::write(const void *buffer, size_t bufferSize, off_t offset, Relative relative) {trace_scope
		off_t	amount;

		AssertMessageException(!_readOnly);
		_goto(offset, relative);
		amount= fwrite(reinterpret_cast<const char*>(buffer), 1, bufferSize, _file);
		AssertMessageException(ferror(_file) == 0);
		AssertMessageException(amount == static_cast<off_t>(bufferSize));
	}
	/**
		@todo Test!
	*/
	inline std::string &File::read(std::string &buffer, size_t bufferSize, off_t offset, Relative relative) const {trace_scope
		if(static_cast<size_t>(-1) == bufferSize) {
			bufferSize= size();
		}
		buffer.assign(bufferSize, '\0');
		read(const_cast<char*>(buffer.data()), bufferSize, offset, relative);
		return buffer;
	}
	/**
		@todo Test!
	*/
	inline void File::write(const std::string &buffer, off_t offset, Relative relative) {trace_scope
		write(buffer.data(), buffer.size(), offset, relative);
	}
	/**
		@todo Test!
	*/
	template<class Int> inline Int File::read(Endian endian, off_t offset, Relative relative) const {trace_scope
		uint8_t	buffer[sizeof(Int)];
		Int		value= 0;

		read(buffer, sizeof(buffer), offset, relative);
		endian= _actualEndian(endian);
		for(unsigned int byte= 0; trace_bool(byte < sizeof(buffer)); ++byte) {
			Int	byteAsInt(buffer[byte]);
			int	shiftAmount;

			if(BigEndian == endian) {
				shiftAmount= 8 * (sizeof(buffer) - byte - 1);
			} else {
				shiftAmount= 8 * byte;
			}
			if(shiftAmount != 0) {
				value |= ( byteAsInt << shiftAmount );
			} else {
				value |= byteAsInt;
			}
		}
		return value;
	}
	/**
		@todo Test!
	*/
	template<class Int> inline void File::write(Int number, Endian endian, off_t offset, Relative relative) {trace_scope
		uint8_t	buffer[sizeof(Int)];

		endian= _actualEndian(endian);
		for(unsigned int byte= 0; trace_bool(byte < sizeof(buffer)); ++byte) {
			int	shiftAmount;

			if(BigEndian == endian) {
				shiftAmount= 8 * (sizeof(buffer) - byte - 1);
			} else {
				shiftAmount= 8 * byte;
			}
			if(shiftAmount != 0) {
				buffer[byte]= 0xFF & ( number >> shiftAmount );
			} else {
				buffer[byte]= 0xFF & number;
			}
		}
		write(buffer, sizeof(buffer), offset, relative);
	}
	inline int File::_whence(Relative relative) {trace_scope
		if(FromHere == relative) {
			return SEEK_CUR;
		} else if(FromStart == relative) {
			return SEEK_SET;
		}
		AssertMessageException(FromEnd == relative);
		return SEEK_END;
	}
	inline void File::_goto(off_t offset, Relative relative) const {trace_scope
		if( trace_bool(0 != offset) || trace_bool(relative != FromHere) ) {
			moveto(offset, relative);
		}
	}
	/**
		@todo Test!
	*/
	inline File::Endian File::_actualEndian(Endian endian) {trace_scope
		if(NativeEndian == endian) {
			uint16_t	value= 1;

			if(*reinterpret_cast<uint8_t*>(&value) == 1) {
				return LittleEndian;
			}
			return BigEndian;
		}
		return endian;
	}
	inline FILE *File::_open(const char *path, File::Method method, File::Protection protection, bool &readOnly) {trace_scope
		FILE		*opened= NULL;
		const bool	tryWritable= ( (ReadWrite == protection) | (WriteIfPossible == protection) );

		if(tryWritable) {
			opened= fopen(path, Binary == method ? kOpenReadWriteBinary : kOpenReadWriteText);
		}
		if( trace_bool(NULL == opened) && trace_bool(tryWritable) ) {
			opened= fopen(path, Binary == method ? kCreateReadWriteBinary : kCreateReadWriteText);
		}
		if(ReadWrite == protection) {
			errnoNULLAssertMessageException(opened);
		}
		if(NULL != opened) {
			readOnly= false;
			return opened;
		}
		opened= fopen(path, Binary == method ? kReadOnlyBinary : kReadOnlyText);
		errnoNULLAssertMessageException(opened);
		readOnly= true;
		return opened;
	}
}

#undef kReadOnlyText
#undef kOpenReadWriteText
#undef kCreateReadWriteText
#undef kReadOnlyBinary
#undef kOpenReadWriteBinary
#undef kCreateReadWriteBinary

#endif // __File_h__
