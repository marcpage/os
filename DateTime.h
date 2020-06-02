#ifndef __DateTime_h__
#define __DateTime_h__

#include "Exception.h"
#include "POSIXErrno.h"
#include <math.h>
#include <string>
#include <sys/time.h>
#include <time.h>

#define AddToConvertToAppleCocoaEpoch                                          \
  (dt::DateTime(2001, dt::DateTime::Jan, 1, dt::DateTime::GMT) -               \
   dt::DateTime(0.0))
#define AddToConvertToDOSEpoch                                                 \
  (dt::DateTime(1980, dt::DateTime::Jan, 1, dt::DateTime::GMT) -               \
   dt::DateTime(0.0))
#define AddToConvertToUNIXEpoch                                                \
  (dt::DateTime(1970, dt::DateTime::Jan, 1, dt::DateTime::GMT) -               \
   dt::DateTime(0.0))
#define AddToConvertToAppleMacEpoch                                            \
  (dt::DateTime(1904, dt::DateTime::Jan, 1, dt::DateTime::GMT) -               \
   dt::DateTime(0.0))

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
  enum Location { GMT, Local };
  typedef std::string String;
  DateTime();
  DateTime(const DateTime &time);
  explicit DateTime(const time_t &time);
  explicit DateTime(const timeval &time);
  explicit DateTime(const timespec &time);
  explicit DateTime(tm &time, Location location = Local);
  explicit DateTime(const double &time);
  DateTime(int year, Month month, int day, int hour24 = 0, int minutes = 0,
           double seconds = 0.0, Location location = Local);
  DateTime(int year, Month month, int day, int hour, CivilianHour ampm,
           int minutes = 0, double seconds = 0.0, Location location = Local);
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
  bool operator>(const DateTime &other) const { return (other < *this); }
  bool operator<=(const DateTime &other) const { return !(other < *this); }
  bool operator>=(const DateTime &other) const { return !(*this < other); }
  double seconds() const;
  DateTime &add(double value, Span span = Seconds);
  tm &gmt(tm &time) const;
  tm &local(tm &time) const;
  timeval &value(timeval &tv) const;
  timespec &value(timespec &ts) const;
  String &format(const String &format, String &buffer) const;
  String format(const String &format) const;

private:
  timespec _time;
  void _init(tm &time, double fractionalSeconds, Location location);
  void _init(int year, Month month, int day, int hour24, int minutes,
             double seconds, Location location);
};
/**
        @todo look at using mach_absolute_time/mach_timebase_info
                        /usr/include/mach/mach_time.h
                        http://developer.apple.com/library/mac/#qa/qa1398/_index.html
*/
inline DateTime::DateTime() : _time() {
  struct timeval timeValue;
  ErrnoOnNegative(::gettimeofday(&timeValue, NULL));
  _time.tv_sec = timeValue.tv_sec;
  _time.tv_nsec = static_cast<long>(timeValue.tv_usec) * 1000UL;
}
inline DateTime::DateTime(const DateTime &time) : _time(time._time) { ; }
inline DateTime::DateTime(const time_t &time) : _time() {
  _time.tv_sec = time;
  _time.tv_nsec = 0UL;
}
inline DateTime::DateTime(const timeval &time) : _time() {
  _time.tv_sec = time.tv_sec;
  _time.tv_nsec = static_cast<long>(time.tv_usec) * 1000UL;
}
inline DateTime::DateTime(const timespec &time) : _time(time) {}
inline void DateTime::_init(tm &time, double fractionalSeconds,
                            Location location) {
  _time.tv_sec = location == GMT ? timegm(&time) : mktime(&time);
  _time.tv_nsec = static_cast<long>(fractionalSeconds * 1000000000.0);
  AssertMessageException(_time.tv_sec != static_cast<time_t>(-1));
}
inline DateTime::DateTime(tm &time, Location location) : _time() {
  _init(time, 0.0, location);
}
inline DateTime::DateTime(const double &time) : _time() {
  double wholeSeconds = floor(time);

  _time.tv_sec = static_cast<time_t>(wholeSeconds);
  _time.tv_nsec = static_cast<long>(1000000000.0 * (time - wholeSeconds));
}
inline void DateTime::_init(int year, Month month, int day, int hour24,
                            int minutes, double secs, Location location) {
  struct tm date;
  double wholeSeconds = floor(secs);

  date.tm_year = year - 1900;
  date.tm_mon = static_cast<int>(month);
  date.tm_mday = day;
  date.tm_hour = hour24;
  date.tm_min = minutes;
  date.tm_sec = static_cast<int>(wholeSeconds);
  _init(date, secs - wholeSeconds, location);
}
inline DateTime::DateTime(int year, Month month, int day, int hour24,
                          int minutes, double secs, Location location)
    : _time() {
  _init(year, month, day, hour24, minutes, secs, location);
}
inline DateTime::DateTime(int year, Month month, int day, int hour,
                          CivilianHour ampm, int minutes, double secs,
                          Location location)
    : _time() {
  if (12 == hour) {
    if (AM == ampm) { // not tested
      hour = 0;       // not tested
    }
  } else if (PM == ampm) {
    hour += 12;
  }
  _init(year, month, day, hour, minutes, secs, location);
}
inline DateTime::~DateTime() { ; }
/// @todo Test
inline DateTime::operator time_t() const { return _time.tv_sec; }
/// @todo Test
inline DateTime::operator timeval() const {
  struct timeval timeValue;
  return value(timeValue);
}
/// @todo Test
inline DateTime::operator const timespec() const { return _time; }
/// @todo Test
inline DateTime::operator double() const { return seconds(); }
inline DateTime &DateTime::operator+=(double secs) {
  *this = *this + secs;
  return *this;
}
inline DateTime DateTime::operator+(double secs) const {
  return DateTime(seconds() + secs);
}
inline DateTime &DateTime::operator-=(double secs) {
  *this = *this - secs;
  return *this;
}
inline DateTime DateTime::operator-(double secs) const {
  return DateTime(seconds() - secs);
}
inline double DateTime::operator-(const DateTime &other) const {
  return seconds() - other.seconds();
}
inline DateTime &DateTime::operator=(const DateTime &other) {
  _time = other._time;
  return *this;
}
inline bool DateTime::operator==(const DateTime &other) const {
  return (_time.tv_sec == other._time.tv_sec) &&
         (_time.tv_nsec == other._time.tv_nsec);
}
inline bool DateTime::operator!=(const DateTime &other) const {
  return (_time.tv_sec != other._time.tv_sec) ||
         (_time.tv_nsec != other._time.tv_nsec);
}
inline bool DateTime::operator<(const DateTime &other) const {
  if (_time.tv_sec == other._time.tv_sec) {
    return _time.tv_nsec < other._time.tv_nsec;
  }
  return _time.tv_sec < other._time.tv_sec;
}
inline double DateTime::seconds() const {
  return static_cast<double>(_time.tv_sec) +
         static_cast<double>(_time.tv_nsec) / 1000000000.0;
}
inline DateTime &DateTime::add(double amount, Span span) {
  switch (span) {
  case Weeks:
    amount *= 7.0 * 24.0 * 60.0 * 60.0;
    break;
  case Days:
    amount *= 24.0 * 60.0 * 60.0;
    break;
  case Hours:
    amount *= 60.0 * 60.0;
    break;
  case Minutes:
    amount *= 60.0;
    break;
  case Seconds:
    break;
  default:
    ThrowMessageException("Unknown timespan"); // not tested
    break;                                     // not tested
  }
  return *this += amount;
}
/// @todo Test
inline tm &DateTime::gmt(tm &time) const {
  return *::gmtime_r(&_time.tv_sec, &time);
}
inline tm &DateTime::local(tm &time) const {
  return *::localtime_r(&_time.tv_sec, &time);
}
/// @todo Test
inline timeval &DateTime::value(timeval &tv) const {
  tv.tv_sec = _time.tv_sec;
  tv.tv_usec = static_cast<suseconds_t>(_time.tv_nsec / 1000UL);
  return tv;
}
/// @todo Test
inline timespec &DateTime::value(timespec &ts) const { return ts = _time; }
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
