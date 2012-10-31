#ifndef __UTIL_TIME_HPP
#define __UTIL_TIME_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>

namespace util
{

class Time
{
  boost::posix_time::ptime now;
  boost::mutex mtx;

  static Time instance;
  Time() { instance.Update(); }
public:

  // need to make this a little bit smarter in order to avoid redudnant calls.
  static void Update();

  static int Day() { return instance.now.date().day(); };
  static int Week() { return instance.now.date().week_number(); };
  static int Month() { return instance.now.date().month(); };
  static int Year() { return instance.now.date().year(); };
};

// end
}
#endif
  

