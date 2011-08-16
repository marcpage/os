#ifndef __tracer_h__
#define __tracer_h__

#include <stdint.h>
#include <stdio.h>

#define VARIABLE 3
#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y)  PASTER(x,y)
#define NAME(fun) EVALUATOR(fun, VARIABLE)

namespace tracer {
	template<typename Type>
	const Type &passThroughLog(const Type &value, const char *expression, const char *file, const char *func, int line) {
		double	doubleValue;
		int64_t	integerValue= value;

		doubleValue= integerValue;
		printf("%s:%d:%s()\t%1.0f (%s)\n", file, line, func, doubleValue, expression);
		return value;
	}
	bool passThroughBoolLog(bool value, const char *expression, const char *file, const char *func, int line) {
		return passThroughLog(value, expression, file, func, line);
	}
	class LogBlock {
		public:
			LogBlock(const char *file, const char *func, int line)
				:_file(file), _func(func), _line(line) {
				printf("%s:%d:%s()\tenter (block)\n", _file, _line, _func);
			}
			~LogBlock() {
				printf("%s:%d:%s()\tleave (block)\n", _file, _line, _func);
			}
		private:
			const char	*_file;
			const char	*_func;
			int			_line;
	};
}

#define if(value) if(tracer::passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
#define while(value) while(tracer::passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
#define switch(value) switch(tracer::passThroughLog((value), #value, __FILE__, __func__, __LINE__))
#define trace_block tracer::LogBlock	EVALUATOR(log_block,__LINE__)(__FILE__,__func__,__LINE__);

#endif // __tracer_h__
