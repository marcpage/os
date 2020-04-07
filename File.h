#ifndef __File_h__
#define __File_h__

#include "Exception.h"
#include "POSIXErrno.h"
#include <string>

#if _DEBUG_FILE // Debug
inline FILE *printResult(FILE *t) {printf("\t RESULT: %08x\n", t); return t;}
inline off_t printResult(off_t t) {printf("\t RESULT: %d\n", t); return t;}
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

	/**
		@todo Document
	*/
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
			std::string &readline(std::string &buffer, off_t offset= 0, Relative relative= FromHere, size_t bufferSize= 4096) const;
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
		:_file(NULL), _readOnly(ReadOnly == protection) {
		_file= _open(path, method, protection, _readOnly);
		moveto(0);
	}
	/**
		@todo Test!
	*/
	inline File::File(const std::string &path, Method method, Protection protection)
		:_file(NULL), _readOnly(ReadOnly == protection) {
		_file= _open(path.c_str(), method, protection, _readOnly);
		moveto(0);
	}
	inline File::~File() {
		if(NULL != _file) {
			fclose(_file);
			_file= NULL;
		}
	}
	/**
		@todo Test!
	*/
	inline off_t File::size() const {
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
	inline void File::flush() {
		ErrnoOnNegative(fflush(_file));
	}
	/**
		@todo Test!
	*/
	inline off_t File::location() const {
		off_t	currentPos;

		ErrnoOnNegative(currentPos= ftello(_file));
		return currentPos;
	}
	/**
		@todo Test!
	*/
	inline bool File::writable() const {
		return (!_readOnly);
	}
	inline void File::moveto(off_t offset, Relative relative) const {
		ErrnoOnNegative(fseeko(_file, offset, _whence(relative)));
	}
	/**
		@todo Test!
	*/
	inline void File::move(off_t offset, Relative relative) const {
		moveto(offset, relative);
	}
	/**
		@todo Test!
	*/
	inline void File::read(void *buffer, size_t bufferSize, off_t offset, Relative relative) const {
		off_t	amount;
		int		fileError;

		_goto(offset, relative);
		AssertMessageException(location() + static_cast<off_t>(bufferSize) <= size());
		amount= fread(reinterpret_cast<char*>(buffer), 1, bufferSize, _file);
		AssertMessageException( ((fileError= ferror(_file)) == 0) || (fileError == EOF) );
		AssertMessageException(amount == static_cast<off_t>(bufferSize));
	}
	inline void File::write(const void *buffer, size_t bufferSize, off_t offset, Relative relative) {
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
	inline std::string &File::read(std::string &buffer, size_t bufferSize, off_t offset, Relative relative) const {
		if(static_cast<size_t>(-1) == bufferSize) {
			bufferSize= size() - offset;
		}
		buffer.assign(bufferSize, '\0');
		read(const_cast<char*>(buffer.data()), bufferSize, offset, relative);
		return buffer;
	}
	/**
		@todo Test!
	*/
	inline void File::write(const std::string &buffer, off_t offset, Relative relative) {
		write(buffer.data(), buffer.size(), offset, relative);
	}
	/**
		@todo Test!
	*/
	template<class Int> inline Int File::read(Endian endian, off_t offset, Relative relative) const {
		uint8_t	buffer[sizeof(Int)];
		Int		value= 0;

		read(buffer, sizeof(buffer), offset, relative);
		endian= _actualEndian(endian);
		for(unsigned int byte= 0; (byte < sizeof(buffer)); ++byte) {
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
	template<class Int> inline void File::write(Int number, Endian endian, off_t offset, Relative relative) {
		uint8_t	buffer[sizeof(Int)];

		endian= _actualEndian(endian);
		for(unsigned int byte= 0; (byte < sizeof(buffer)); ++byte) {
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
	/**
		@todo improve performance by resizing buffer and using read(void*) to read directly into appending buffer
	*/
	inline std::string &File::readline(std::string &buffer, off_t offset, Relative relative, size_t bufferSize) const {
		std::string				partial;
		std::string::size_type	lf, cr, eol;
		off_t					left;
		bool					foundCR= false, foundLF= false;

		buffer.clear();
		_goto(offset, relative);
		left= size() - location();
		while( !foundCR && !foundLF && (left > 0) ) {
			bufferSize= bufferSize > static_cast<size_t>(left) ? static_cast<size_t>(left) : bufferSize;
			read(partial, bufferSize, 0, FromHere);
			left-= partial.size();
			cr= partial.find('\r');
			foundCR= (cr != std::string::npos);
			if(foundCR && (cr == partial.size() - 1) && (left > 0) ) {
				char	character;

				read(&character, 1, 0, FromHere);
				partial.append(1, character);
			}
			lf= partial.find('\n');
			foundLF= (lf != std::string::npos);
			eol= std::string::npos;
			if(foundLF && foundCR) {
				eol= ( (cr + 1 == lf) || (lf < cr) ) ? lf : cr;
			} else if(foundLF || foundCR) {
				eol= foundLF ? lf : cr;
			}
			if(eol != std::string::npos) {
				const off_t	rewind= (partial.size() - eol - 1);

				_goto(-1 * rewind, FromHere);
				partial.erase(eol + 1);
			}
			buffer.append(partial);
		}
		return buffer;
	}
	inline int File::_whence(Relative relative) {
		if(FromHere == relative) {
			return SEEK_CUR;
		} else if(FromStart == relative) {
			return SEEK_SET;
		}
		AssertMessageException(FromEnd == relative);
		return SEEK_END;
	}
	inline void File::_goto(off_t offset, Relative relative) const {
		if( (0 != offset) || (relative != FromHere) ) {
			moveto(offset, relative);
		}
	}
	/**
		@todo Test!
	*/
	inline File::Endian File::_actualEndian(Endian endian) {
		if(NativeEndian == endian) {
			uint16_t	value= 1;

			if(*reinterpret_cast<uint8_t*>(&value) == 1) {
				return LittleEndian;
			}
			return BigEndian;
		}
		return endian;
	}
	inline FILE *File::_open(const char *path, File::Method method, File::Protection protection, bool &readOnly) {
		FILE		*opened= NULL;
		const bool	tryWritable= ( (ReadWrite == protection) | (WriteIfPossible == protection) );

		if(tryWritable) {
			opened= fopen(path, Binary == method ? kOpenReadWriteBinary : kOpenReadWriteText);
		}
		if( (NULL == opened) && (tryWritable) ) {
			opened= fopen(path, Binary == method ? kCreateReadWriteBinary : kCreateReadWriteText);
		}
		if(ReadWrite == protection) {
			ErrnoOnNULL(opened);
		}
		if(NULL != opened) {
			readOnly= false;
			return opened;
		}
		opened= fopen(path, Binary == method ? kReadOnlyBinary : kReadOnlyText);
		ErrnoOnNULL(opened);
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
