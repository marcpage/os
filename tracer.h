#ifndef __tracer_h__
#define __tracer_h__

#include <stdint.h>
#include <stdio.h>

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

#define if(value) if(passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
#define while(value) while(passThroughBoolLog((value), #value, __FILE__, __func__, __LINE__))
#define switch(value) switch(passThroughLog((value), #value, __FILE__, __func__, __LINE__))

#endif // __tracer_h__
