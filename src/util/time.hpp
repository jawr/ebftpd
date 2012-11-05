#ifndef __UTIL_TIME_HPP
#define __UTIL_TIME_HPP

#include <boost/date_time/posix_time/posix_time.hpp>

namespace util
{

class Time
{
  boost::posix_time::ptime now;

public:
  Time() { now = boost::posix_time::second_clock::local_time(); }

  int Day() { return now.date().day(); };
  int Week() { return now.date().week_number(); };
  int Month() { return now.date().month(); };
  int Year() { return now.date().year(); };
};

// end
}
#endif
  

