#ifndef __UTIL_TIME_HPP
#define __UTIL_TIME_HPP

#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace util
{

class Time
{
  boost::posix_time::ptime t;

public:
  Time(time_t t) : t(boost::posix_time::from_time_t(t)) { }
  Time() : t(boost::posix_time::second_clock::local_time()) { }

  int Day() { return t.date().day(); };
  int Week() { return t.date().week_number(); };
  int Month() { return t.date().month(); };
  int Year() { return t.date().year(); };
};

// end
}
#endif
  

