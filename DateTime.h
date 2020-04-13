#ifndef __DateTime_h__
#define __DateTime_h__

#include "Exception.h"
#include "POSIXErrno.h"
#include <math.h>
#include <string>
#include <sys/time.h>
#include <time.h>

#define AddToConvertToAppleCocoaEpoch                                          \
  (dt::DateTime(2001, dt::DateTime::Jan, 1) - dt::DateTime(0.0))
#define AddToConvertToDOSEpoch                                                 \
  (dt::DateTime(1980, dt::DateTime::Jan, 1) - dt::DateTime(0.0))
#define AddToConvertToUNIXEpoch                                                \
  (dt::DateTime(1970, dt::DateTime::Jan, 1) - dt::DateTime(0.0))
#define AddToConvertToAppleMacEpoch                                            \
  (dt::DateTime(1904, dt::DateTime::Jan, 1) - dt::DateTime(0.0))

namespace dt {

/**
        @todo Document DateTime
*/
class DateTime {
public:
  enum Span { Seconds, Minutes, Hours, Days, Weeks };
  enum Month {
    Jan,
    January = Jan,
    Feb,
    February = Feb,
    Mar,
    March = Mar,
    Apr,
    April = Apr,
    May,
    Jun,
    June = Jun,
    Jul,
    July = Jul,
    Aug,
    August = Aug,
    Sep,
    September = Sep,
    Oct,
    October = Oct,
    Nov,
    November = Nov,
    Dec,
    December = Dec,
  };
  enum CivilianHour { AM, PM };
  typedef std::string String;
  DateTime();
  DateTime(const DateTime &time);
  explicit DateTime(const time_t &time);
  explicit DateTime(const timeval &time);
  explicit DateTime(const timespec &time);
  explicit DateTime(tm &time);
  explicit DateTime(const double &time);
  DateTime(int year, Month month, int day, int hour24 = 0, int minutes = 0,
           double seconds = 0.0);
  DateTime(int year, Month month, int day, int hour, CivilianHour ampm,
           int minutes = 0, double seconds = 0.0);
  ~DateTime();
  operator time_t() const;
  operator timeval() const;
  operator const timespec() const;
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
  DateTime &add(double value, Span span = Seconds);
  tm &utc(tm &time) const;
  tm &local(tm &time) const;
  timeval &value(timeval &tv) const;
  timespec &value(timespec &ts) const;
  String &format(const String &format, String &buffer) const;
  String format(const String &format) const;

private:
  timespec _time;
  void _init(tm &time, double fractionalSeconds = 0.0);
  void _init(int year, Month month, int day, int hour24, int minutes,
             double seconds);
};
/**
        @todo look at using mach_absolute_time/mach_timebase_info
                        /usr/include/mach/mach_time.h
                        http://developer.apple.com/library/mac/#qa/qa1398/_index.html
*/
inline DateTime::DateTime() : _time() {
  ;
  struct timeval timeValue;
  ErrnoOnNegative(::gettimeofday(&timeValue, NULL));
  _time.tv_sec = timeValue.tv_sec;
  _time.tv_nsec = static_cast<long>(timeValue.tv_usec) * 1000UL;
}
inline DateTime::DateTime(const DateTime &time) : _time(time._time) { ; }
inline DateTime::DateTime(const time_t &time) : _time() {
  ;
  _time.tv_sec = time;
  _time.tv_nsec = 0UL;
}
inline DateTime::DateTime(const timeval &time) : _time() {
  ;
  _time.tv_sec = time.tv_sec;
  _time.tv_nsec = static_cast<long>(time.tv_usec) * 1000UL;
}
inline DateTime::DateTime(const timespec &time) : _time(time) {}
inline void DateTime::_init(tm &time, double fractionalSeconds) {
  _time.tv_sec = mktime(&time);
  _time.tv_nsec = static_cast<long>(fractionalSeconds * 1000000000.0);
  AssertMessageException(_time.tv_sec != static_cast<time_t>(-1));
}
inline DateTime::DateTime(tm &time) : _time() {
  ;
  _init(time);
}
inline DateTime::DateTime(const double &time) : _time() {
  ;
  double wholeSeconds = floor(time);

  _time.tv_sec = static_cast<time_t>(wholeSeconds);
  _time.tv_nsec = static_cast<long>(1000000000.0 * (time - wholeSeconds));
}
inline void DateTime::_init(int year, Month month, int day, int hour24,
                            int minutes, double secs) {
  struct tm date;
  double wholeSeconds = floor(secs);

  date.tm_year = year - 1900;
  date.tm_mon = static_cast<int>(month);
  date.tm_mday = day;
  date.tm_hour = hour24;
  date.tm_min = minutes;
  date.tm_sec = static_cast<int>(wholeSeconds);
  _init(date, secs - wholeSeconds);
}
inline DateTime::DateTime(int year, Month month, int day, int hour24,
                          int minutes, double secs)
    : _time() {
  _init(year, month, day, hour24, minutes, secs);
}
inline DateTime::DateTime(int year, Month month, int day, int hour,
                          CivilianHour ampm, int minutes, double secs)
    : _time() {
  if (12 == hour) {
    if (AM == ampm) {
      hour = 0;
    }
  } else if (PM == ampm) {
    hour += 12;
  }
  _init(year, month, day, hour, minutes, secs);
}
inline DateTime::~DateTime() { ; }
inline DateTime::operator time_t() const {
  ;
  return _time.tv_sec;
}
inline DateTime::operator timeval() const {
  ;
  struct timeval timeValue;
  value(timeValue);
  return timeValue;
}
inline DateTime::operator const timespec() const { return _time; }
inline DateTime::operator double() const {
  ;
  return seconds();
}
inline DateTime &DateTime::operator+=(double secs) {
  ;
  *this = *this + secs;
  return *this;
}
inline DateTime DateTime::operator+(double secs) const {
  ;
  return DateTime(seconds() + secs);
}
inline DateTime &DateTime::operator-=(double secs) {
  ;
  *this = *this - secs;
  return *this;
}
inline DateTime DateTime::operator-(double secs) const {
  ;
  return DateTime(seconds() - secs);
}
inline double DateTime::operator-(const DateTime &other) const {
  ;
  return seconds() - other.seconds();
}
inline DateTime &DateTime::operator=(const DateTime &other) {
  ;
  _time = other._time;
  return *this;
}
inline bool DateTime::operator==(const DateTime &other) const {
  ;
  return (_time.tv_sec == other._time.tv_sec) &&
         (_time.tv_nsec == other._time.tv_nsec);
}
inline bool DateTime::operator!=(const DateTime &other) const {
  ;
  return (_time.tv_sec != other._time.tv_sec) ||
         (_time.tv_nsec != other._time.tv_nsec);
}
inline bool DateTime::operator<(const DateTime &other) const {
  ;
  if (_time.tv_sec == other._time.tv_sec) {
    return _time.tv_nsec < other._time.tv_nsec;
  }
  return _time.tv_sec < other._time.tv_sec;
}
inline bool DateTime::operator>(const DateTime &other) const {
  ;
  if (_time.tv_sec == other._time.tv_sec) {
    return _time.tv_nsec > other._time.tv_nsec;
  }
  return _time.tv_sec > other._time.tv_sec;
}
inline bool DateTime::operator<=(const DateTime &other) const {
  ;
  if (_time.tv_sec == other._time.tv_sec) {
    return _time.tv_nsec <= other._time.tv_nsec;
  }
  return _time.tv_sec <= other._time.tv_sec;
}
inline bool DateTime::operator>=(const DateTime &other) const {
  ;
  if (_time.tv_sec == other._time.tv_sec) {
    return _time.tv_nsec >= other._time.tv_nsec;
  }
  return _time.tv_sec >= other._time.tv_sec;
}
inline double DateTime::seconds() const {
  ;
  return static_cast<double>(_time.tv_sec) +
         static_cast<double>(_time.tv_nsec) / 1000000000.0;
}
inline DateTime &DateTime::add(double amount, Span span) {
  ;
  switch (span) {
  case Weeks:
    amount *= 7.0;
  case Days:
    amount *= 24.0;
  case Hours:
    amount *= 60.0;
  case Minutes:
    amount *= 60.0;
  case Seconds:
    break;
  default:
    break;
  }
  return *this += amount;
}
inline tm &DateTime::utc(tm &time) const {
  ;
  return *::localtime_r(&_time.tv_sec, &time);
}
inline tm &DateTime::local(tm &time) const {
  ;
  return *::gmtime_r(&_time.tv_sec, &time);
}
inline timeval &DateTime::value(timeval &tv) const {
  tv.tv_sec = _time.tv_sec;
  tv.tv_usec = static_cast<suseconds_t>(_time.tv_nsec / 1000UL);
  return tv;
}
inline timespec &DateTime::value(timespec &ts) const {
  ts = _time;
  return ts;
}
inline DateTime::String &DateTime::format(const String &format,
                                          String &buffer) const {
  size_t size;
  struct tm timeValue;

  local(timeValue);
  buffer.assign(format.length() * 15, '\0');
  size = ::strftime(const_cast<char *>(buffer.data()), buffer.length(),
                    format.c_str(), &timeValue);
  buffer.erase(size);
  return buffer;
}
inline DateTime::String DateTime::format(const String &fmt) const {
  String buffer;

  return format(fmt, buffer);
}
} // namespace dt

#endif // __DateTime_h__
