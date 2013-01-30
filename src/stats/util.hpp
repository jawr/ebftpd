#ifndef __STATS_UTIL_HPP
#define __STATS_UTIL_HPP

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include <cmath>

namespace ftp
{
class Client;
}

namespace fs
{
class VirtualPath;
}

namespace cfg
{
class Section;
}

namespace stats
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

inline boost::posix_time::time_duration SpeedLimitSleep(double speed, double speedLimit)
{
  if (speed <= speedLimit) return boost::posix_time::microseconds(0);
  return boost::posix_time::microseconds(std::ceil(speed / speedLimit * 1000000.0));
}
        
std::string AutoUnitSpeedString(double speed);
std::string AutoUnitString(double amount);
std::string HighResSecondsString(const boost::posix_time::time_duration& duration);
inline std::string HighResSecondsString(const boost::posix_time::ptime& start, 
        const boost::posix_time::ptime& end)
{
  return HighResSecondsString(end - start);
}

int UploadRatio(const ftp::Client& client, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section);
int DownloadRatio(const ftp::Client& client, const fs::VirtualPath& path, 
    const boost::optional<const cfg::Section&>& section);

} /* stats namespace */

#endif
