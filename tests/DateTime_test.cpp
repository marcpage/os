#include "os/DateTime.h"
#include <stdio.h>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

int main(const int /*argc*/, const char *const /*argv*/[]) {
  int iterations = 15000;
#ifdef __Tracer_h__
  iterations = 1;
#endif
  for (int i = 0; i < iterations; ++i) {
    dt::DateTime t1, t2;
    std::string buffer;

    fprintf(stderr, "construction time: %0.9f\n", t2 - t1);
    fprintf(stderr, "Years to Cocoa Epoch %0.1f\n",
            AddToConvertToAppleCocoaEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
    fprintf(stderr, "Years to DOS Epoch %0.1f\n",
            AddToConvertToDOSEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
    fprintf(stderr, "Years to UNIX Epoch %0.1f\n",
            AddToConvertToUNIXEpoch / 365.2525 / 24.0 / 60.0 / 60.0);
    fprintf(stderr, "Years to Apple Epoch %0.1f\n",
            AddToConvertToAppleMacEpoch / 365.2525 / 24.0 / 60.0 / 60.0);

    t1 = dt::DateTime(2018, dt::DateTime::Feb, 5, 14, 30, 15);
    dt::DateTime copy1(t1);

    dotest(t1 == copy1);
    dotest(t1.format("%A", buffer) == "Monday");
    dotest(t1.format("%a", buffer) == "Mon");
    dotest(t1.format("%B", buffer) == "February");
    dotest(t1.format("%b", buffer) == "Feb");
    dotest(t1.format("%C", buffer) == "20");
    dotest(t1.format("%D", buffer) == "02/05/18");
    dotest(t1.format("%d", buffer) == "05");
    dotest(t1.format("%F", buffer) == "2018-02-05");
    dotest(t1.format("%M", buffer) == "30");
    dotest(t1.format("%m", buffer) == "02");
    dotest(t1.format("%p", buffer) == "PM");
    dotest(t1.format("%S", buffer) == "15");

    time_t now;

    time(&now);

    dotest(dt::DateTime() - dt::DateTime(now) < 1.0);

    struct timeval timeValue;
    ErrnoOnNegative(::gettimeofday(&timeValue, NULL));

    dotest(dt::DateTime() - dt::DateTime(timeValue) < 1.0);

    struct tm date;

    date.tm_year = 120;
    date.tm_mon = 5;
    date.tm_mday = 28;
    date.tm_hour = 20;
    date.tm_min = 57;
    date.tm_sec = 54;
    const char *dateFormat = "%Y/%m/%d %H:%M:%S";

    printf(
        "%s vs %s\n",
        dt::DateTime(date, dt::DateTime::GMT).format(dateFormat).c_str(),
        dt::DateTime(2020, dt::DateTime::May, 28, 20, 57, 54, dt::DateTime::GMT)
            .format(dateFormat)
            .c_str());
    dotest(dt::DateTime(date, dt::DateTime::GMT) -
               dt::DateTime(2020, dt::DateTime::June, 28, 20, 57, 54,
                            dt::DateTime::GMT) <
           1.0);
    dotest(dt::DateTime(date, dt::DateTime::GMT) -
               dt::DateTime(2020, dt::DateTime::June, 28, 8, dt::DateTime::PM,
                            57, 54, dt::DateTime::GMT) <
           1.0);

    dotest(dt::DateTime() - ((dt::DateTime() + 5.0) - 5.0) < 1.0);
    dotest(dt::DateTime() - ((dt::DateTime() + 5.0) - 5.0) > -1.0);

    dotest(dt::DateTime() - (dt::DateTime() -= 5.0) > 4.9);
    dotest((dt::DateTime() += 5.0) - dt::DateTime() > 4.9);

    dotest(dt::DateTime(2020, dt::DateTime::June, 28, 20, 57, 54,
                        dt::DateTime::GMT) ==
           dt::DateTime(2020, dt::DateTime::June, 28, 8, dt::DateTime::PM, 57,
                        54, dt::DateTime::GMT))

        dotest(dt::DateTime(2020, dt::DateTime::June, 28, 8, 57, 54,
                            dt::DateTime::GMT) ==
               dt::DateTime(2020, dt::DateTime::June, 28, 8, dt::DateTime::AM,
                            57, 54, dt::DateTime::GMT))

            dotest((dt::DateTime() + 0.001) != dt::DateTime());
    dotest((dt::DateTime() += 1.0) > dt::DateTime());
    dotest((dt::DateTime() -= 1.0) < dt::DateTime());
    dotest((dt::DateTime() += 1.0) >= dt::DateTime());
    dotest((dt::DateTime() -= 1.0) <= dt::DateTime());

    dotest((dt::DateTime() += -0.1) <= dt::DateTime());
    dotest((dt::DateTime() -= 0.1) <= dt::DateTime());

    dotest((dt::DateTime() += 0.1) >= dt::DateTime());
    dotest((dt::DateTime() -= -0.1) >= dt::DateTime());

    dotest(fabs(dt::DateTime().add(1.0, dt::DateTime::Seconds) -
                (dt::DateTime() + 1.0)) < 1.0);
    dotest(fabs(dt::DateTime().add(1.0, dt::DateTime::Minutes) -
                (dt::DateTime() + 60.0)) < 1.0);
    dotest(fabs(dt::DateTime().add(1.0, dt::DateTime::Hours) -
                (dt::DateTime() + 3600.0)) < 1.0);
    dotest(fabs(dt::DateTime().add(1.0, dt::DateTime::Days) -
                (dt::DateTime() + 24.0 * 3600.0)) < 1.0);
    dotest(fabs(dt::DateTime().add(1.0, dt::DateTime::Weeks) -
                (dt::DateTime() + 7.0 * 24.0 * 3600.0)) < 1.0);
  }
  return 0;
}
