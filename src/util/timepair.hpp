#ifndef __UTIL_TIMEPAIR_HPP
#define __UTIL_TIMEPAIR_HPP

#include <sys/time.h>
#include <ctime>
#include <cmath>

namespace util
{

class TimePair
{
	struct timeval pair;

public:
	TimePair(time_t seconds, suseconds_t microseconds)
	{
		pair.tv_sec = seconds;
		pair.tv_usec = microseconds;
	}
  
  TimePair(double duration)
  {
    pair.tv_sec = round(duration);
    pair.tv_usec = (duration - pair.tv_sec) * 1000000;
  }
  
  time_t Seconds() const { return pair.tv_sec; }
  suseconds_t Microseconds() const { return pair.tv_usec; }
  const struct timeval& Timeval() const { return pair; }
  struct timeval& Timeval() { return pair; }
  
  double ToDouble() const
  { return pair.tv_usec / 1000000.0 + pair.tv_sec; }
  
  static TimePair Seconds(time_t seconds)
  { return TimePair(seconds, 0); }
  static TimePair Microseconds(suseconds_t microseconds)
  { return TimePair(0, microseconds); }
};

} /* util namespace */

#endif
