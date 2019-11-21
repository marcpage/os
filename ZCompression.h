#ifndef __ZCompression_h__
#define __ZCompression_h__

#include <zlib.h>
#include "Exception.h"

#define zlib_handle_error(code) if(0 != (code)) throw z::Exception(code, __FILE__, __LINE__); else z::noop()

namespace z {

	inline void noop() {}

	class Exception : public msg::Exception {
		public:
			typedef msg::Exception	Super;
			Exception(int zcode, const char *file= NULL, int line= 0) throw();
			Exception(const Exception &other);
			Exception &operator=(const Exception &other);
			virtual ~Exception() throw();
			virtual const char* what() const throw();
			int code();
		private:
			int	_code;
			static const char *_codestring(int code);
	};

	inline Exception::Exception(int zcode, const char *file, int line) throw()
		:Super(_codestring(zcode), file, line), _code(zcode) {
	}
	inline Exception::Exception(const Exception &other)
		:Super(other), _code(other._code) {
	}
	inline Exception &Exception::operator=(const Exception &other) {
		Super::operator=(other);
		_code= other._code;
		return *this;
	}
	inline Exception::~Exception() throw() {
	}
	inline const char* Exception::what() const throw() {
		return Super::what();
	}
	inline int Exception::code() {
		return _code;
	}
	inline const char *Exception::_codestring(int code) {
		switch(code) {
			case Z_OK:
				break;
			case Z_MEM_ERROR:
				return "Out of Memory";
			case Z_BUF_ERROR:
				return "Destination was not big enough";
			case Z_STREAM_ERROR:
				return "level was not valid";
			case Z_DATA_ERROR:
				return "Corrupt compressed data";
			default:
				return "Unknown error code";
		}
		return "";
	}

	inline size_t compress(const void *source, size_t sourceSize, void *destination, size_t destinationSize, int level=6) {
		uLong	dSize= destinationSize;
		AssertMessageException(destinationSize >= ::compressBound(sourceSize));

		zlib_handle_error(::compress2(reinterpret_cast<Bytef*>(destination), &dSize,
									reinterpret_cast<const Bytef*>(source), static_cast<uLong>(sourceSize), level));
		return dSize;
	}

	inline std::string &compress(const std::string &source, std::string &destination, int level= 6) {
		const size_t	maxDestination= ::compressBound(source.size());

		destination.assign(static_cast<std::string::size_type>(maxDestination), '\0');

		const size_t	actualDestination= compress(source.data(), source.size(), const_cast<char*>(destination.data()), maxDestination, level);

		destination.resize(static_cast<std::string::size_type>(actualDestination));
		return destination;
	}

	inline std::string compress(const std::string &source, int level=6) {
		std::string buffer;

		return compress(source, buffer, level);
	}

	inline size_t uncompress(const void *source, size_t sourceSize, void *destination, size_t destinationSize) {
		zlib_handle_error(::uncompress(reinterpret_cast<Bytef*>(destination), static_cast<uLong*>(&destinationSize),
										const_cast<Bytef*>(reinterpret_cast<const Bytef*>(source)), static_cast<uLong>(sourceSize)));
		return destinationSize;
	}

	inline std::string &uncompress(const std::string &source, std::string &destination, std::string::size_type maxDestination= 512 * 1024) {
		destination.assign(maxDestination, '\0');

		const size_t	actualDestination= uncompress(source.data(), source.size(), const_cast<char*>(destination.data()), destination.size());

		destination.resize(static_cast<std::string::size_type>(actualDestination));
		return destination;
	}

	inline std::string uncompress(const std::string &source, std::string::size_type maxDestination= 512 * 1024) {
		std::string results;

		if (source.size() > maxDestination) {
			maxDestination= 2 * source.size();
		}

		uncompress(source, results, maxDestination);
		return results;
	}

};

#endif // __ZCompression_h__
