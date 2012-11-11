#ifndef __STATS_UTIL_HPP
#define __STATS_UTIL_HPP

#include <boost/date_time/posix_time/posix_time.hpp>

namespace stats { namespace util
{

inline double CalculateSpeed(long long bytes, const boost::posix_time::time_duration& duration)
{
  double seconds = duration.total_microseconds() / 1000000.0;
  return seconds == 0.0 ? bytes : bytes / seconds;
}

inline double CalculateSpeed(long long bytes, const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end)
{
  return CalculateSpeed(bytes, end - start);
}
        
std::string AutoUnitSpeedString(double speed);
std::string AutoUnitString(double amount);
std::string HighResSecondsString(const boost::posix_time::time_duration& duration);
inline std::string HighResSecondsString(const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end)
{
  return HighResSecondsString(end - start);
}

} /* util namespace */
} /* stats namespace */

#endif
