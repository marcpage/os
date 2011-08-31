#ifndef __DateTime_h__
#define __DateTime_h__

#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "Exception.h"

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

namespace dt {

	/**
		@todo Document DateTime
	*/
	class DateTime {
		public:
			enum Span {
				Seconds,
				Minutes,
				Hours,
				Days,
				Weeks
			};
			DateTime();
			DateTime(const DateTime &time);
			DateTime(const time_t &time);
			DateTime(const timeval &time);
			DateTime(tm &time);
			DateTime(const double &time);
			~DateTime();
			operator time_t() const;
			operator const timeval&() const;
			operator const timeval*() const;
			operator double() const;
			DateTime &operator+=(double secs);
			DateTime operator+(double secs) const;
			DateTime &operator-=(double secs);
			DateTime operator-(double secs) const;
			double operator-(const DateTime &other) const;
			DateTime &operator=(const DateTime &other);
			bool operator==(const DateTime &other) const;
			bool operator!=(const DateTime &other) const;
			bool operator<(const DateTime &other) const;
			bool operator>(const DateTime &other) const;
			bool operator<=(const DateTime &other) const;
			bool operator>=(const DateTime &other) const;
			double seconds() const;
			DateTime &add(double value, Span span= Seconds);
			tm &utc(tm &time) const;
			tm &local(tm &time) const;
		private:
			timeval	_time;
	};
	inline DateTime::DateTime()
		:_time() {trace_scope;
		errnoAssertMessageException(gettimeofday(&_time,NULL));
	}
	inline DateTime::DateTime(const DateTime &time)
		:_time(time._time) {trace_scope;
	}
	inline DateTime::DateTime(const time_t &time)
		:_time() {trace_scope;
		_time.tv_sec= time;
		_time.tv_usec= 0;
	}
	inline DateTime::DateTime(const timeval &time)
		:_time(time) {trace_scope;
	}
	inline DateTime::DateTime(tm &time)
		:_time() {trace_scope;
		_time.tv_sec= mktime(&time);
		_time.tv_usec= 0;
		AssertMessageException(_time.tv_sec != static_cast<time_t>(-1));
	}
	inline DateTime::DateTime(const double &time)
		:_time() {trace_scope;
		_time.tv_sec= static_cast<time_t>(time);
		_time.tv_usec= static_cast<suseconds_t>(1000000.0 * (time - floor(time)));
	}
	inline DateTime::~DateTime() {trace_scope;
	}
	inline DateTime::operator time_t() const {trace_scope;
		return _time.tv_sec;
	}
	inline DateTime::operator const timeval&() const {trace_scope;
		return _time;
	}
	inline DateTime::operator const timeval*() const {trace_scope;
		return &_time;
	}
	inline DateTime::operator double() const {trace_scope;
		return seconds();
	}
	inline DateTime &DateTime::operator+=(double secs) {trace_scope;
		*this= *this + secs;
		return *this;
	}
	inline DateTime DateTime::operator+(double secs) const {trace_scope;
		return DateTime(seconds() + secs);
	}
	inline DateTime &DateTime::operator-=(double secs) {trace_scope;
		*this= *this - secs;
		return *this;
	}
	inline DateTime DateTime::operator-(double secs) const {trace_scope;
		return DateTime(seconds() - secs);
	}
	inline double DateTime::operator-(const DateTime &other) const {trace_scope;
		return seconds() - other.seconds();
	}
	inline DateTime &DateTime::operator=(const DateTime &other) {trace_scope;
		_time= other._time;
		return *this;
	}
	inline bool DateTime::operator==(const DateTime &other) const {trace_scope;
		return (_time.tv_sec == other._time.tv_sec)
				&& (_time.tv_usec == other._time.tv_usec);
	}
	inline bool DateTime::operator!=(const DateTime &other) const {trace_scope;
		return (_time.tv_sec != other._time.tv_sec)
				|| (_time.tv_usec != other._time.tv_usec);
	}
	inline bool DateTime::operator<(const DateTime &other) const {trace_scope;
		if(_time.tv_sec == other._time.tv_sec) {
			return _time.tv_usec < other._time.tv_usec;
		}
		return _time.tv_sec < other._time.tv_sec;
	}
	inline bool DateTime::operator>(const DateTime &other) const {trace_scope;
		if(_time.tv_sec == other._time.tv_sec) {
			return _time.tv_usec > other._time.tv_usec;
		}
		return _time.tv_sec > other._time.tv_sec;
	}
	inline bool DateTime::operator<=(const DateTime &other) const {trace_scope;
		if(_time.tv_sec == other._time.tv_sec) {
			return _time.tv_usec <= other._time.tv_usec;
		}
		return _time.tv_sec <= other._time.tv_sec;
	}
	inline bool DateTime::operator>=(const DateTime &other) const {trace_scope;
		if(_time.tv_sec == other._time.tv_sec) {
			return _time.tv_usec >= other._time.tv_usec;
		}
		return _time.tv_sec >= other._time.tv_sec;
	}
	inline double DateTime::seconds() const {trace_scope;
		return static_cast<double>(_time.tv_sec) + static_cast<double>(_time.tv_usec) / 1000000.0;
	}
	inline DateTime &DateTime::add(double value, Span span) {trace_scope;
		switch(span) {
			case Weeks:
				value*= 7.0;
			case Days:
				value*= 24.0;
			case Hours:
				value*= 60.0;
			case Minutes:
				value*= 60.0;
			case Seconds:
				break;
			default:
				break;
		}
		return *this += value;
	}
	inline tm &DateTime::utc(tm &time) const {trace_scope;
		return *localtime_r(&_time.tv_sec, &time);
	}
	inline tm &DateTime::local(tm &time) const {trace_scope;
		return *gmtime_r(&_time.tv_sec, &time);
	}
}

#endif // __DateTime_h__
