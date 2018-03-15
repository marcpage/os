#ifndef __Tracer_h__
#define __Tracer_h__

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

/// Utility Macro to work preprocessor magic, do not use directly
#define PASTER(x,y) __ ## x ## _ ## y
/// Creates a unique variable name (per line)
#define UNIQUE_VARIABLE_NAME(x,y)  PASTER(x,y)

/// functions and classes for tracing
namespace Tracer {
	typedef int (*FormatPrintFunction)(const char *format,...);
	static FormatPrintFunction	formatPrint= printf;
	static inline const char *format_print(std::string &buffer, size_t size, const char * const format, ...) {
		va_list	args;

		buffer.assign(size, '\0');
		va_start(args, format);

		size= vsnprintf(const_cast<char*>(buffer.data()), size, format, args);

		va_end(args);
		buffer.erase(size);
		return buffer.c_str();
	}
	/** Prints to stdout the file, line, function, value (as a double) and the expression.
		This function can be called around any value or expression, and it will return the value of the expression inline.
		@param value		The value to print.
		@param expression	The expression that evaluated to <code>value</code>.
		@param file			The file that <code>expression</code> was in.
		@param func			The function or method that <code>expression</code> was in.
		@param line			The line in <code>file</code> that <code>expression</code> can be found.
		@return				A const reference to <code>value</code>.
	*/
	template<typename Type>
	const Type &passThroughLog(const Type &value, const char *expression, const char *file, const char *func, int line) {
		double		doubleValue;
		int64_t		integerValue= value;
		std::string	buffer;

		doubleValue= integerValue;
		formatPrint("%s",format_print(buffer, 4096, "%s:%d:%s()\t%1.0f (%s)\n", file, line, func, doubleValue, expression));
		return value;
	}
	/** Prints to stdout the file, line, function, value (as a double) and the expression.
		This function can be called around any value or expression, and it will return the value of the expression inline.
		@param value		The value to print.
		@param expression	The expression that evaluated to <code>value</code>.
		@param file			The file that <code>expression</code> was in.
		@param func			The function or method that <code>expression</code> was in.
		@param line			The line in <code>file</code> that <code>expression</code> can be found.
		@return				A const reference to <code>value</code>.
	*/
	bool passThroughBoolLog(bool value, const char *expression, const char *file, const char *func, int line) {
		return passThroughLog(value, expression, file, func, line);
	}
	/** Helper class to log entry and exit of a C scope.
		This class is meant to be implemented right after the opening brace of a scope.
	*/
	class LogBlock {
		public:
			/** Print to stdout that we are entering a scope and remember the scope entry info.
				@param file			The file that we entered scope in.
				@param func			The function or method that we entered scope.
				@param line			The line in <code>file</code> that we entered scope.
			*/
			LogBlock(const char *file, const char *func, int line)
				:_file(file), _func(func), _line(line) {
				std::string	buffer;

				formatPrint("%s",format_print(buffer, 4096, "%s:%d:%s()\tenter (scope)\n", _file, _line, _func));
			}
			/** Print to stdout that we are exiting the scope we entered on construction.
			*/
			~LogBlock() {
				try {
					std::string	buffer;

					formatPrint("%s",format_print(buffer, 4096, "%s:%d:%s()\tleave (scope)\n", _file, _line, _func));
				} catch(const std::exception &exception) {
					formatPrint("Exception thrown in ~LogBlock:%s:%s():%s\n",_file,_func,exception.what());
				}
			}
		private:
			const char	*_file;	///< The file that we entered scope in.
			const char	*_func;	///< The function or method that we entered scope.
			int			_line;	///< The line in <code>_file</code> that we entered scope.
			LogBlock(const LogBlock &other); ///<  prevent usage
			LogBlock &operator=(const LogBlock &other); ///<  prevent usage
	};
}

/// Do a trace on all 'if' statements
#define if(value) if(Tracer::passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
/// Do a trace on all 'while' statements
#define while(value) while(Tracer::passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
/// Do a trace on all 'switch' statements
#define switch(value) switch(Tracer::passThroughLog((value), #value, __FILE__, __func__, __LINE__))
/// Do a trace on any requested scope
#define trace_scope Tracer::LogBlock	UNIQUE_VARIABLE_NAME(log_scope,__LINE__)(__FILE__,__func__,__LINE__);
#define trace_bool(exp) Tracer::passThroughBoolLog((exp), #exp, __FILE__, __func__, __LINE__)
#define trace_value(value) Tracer::passThroughLog((value), #value, __FILE__, __func__, __LINE__)

#endif // __Tracer_h__
