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

inline boost::posix_time::time_duration SpeedLimitSleep(
      const boost::posix_time::time_duration& xfertime, 
      long long bytes, long limit)
{
  auto minXfertime = boost::posix_time::microseconds((bytes / static_cast<double>(limit)) * 1000000);
  if (minXfertime < xfertime) return boost::posix_time::microseconds(0);
  return std::min<boost::posix_time::time_duration>(boost::posix_time::
          microseconds(100000), minXfertime - xfertime);
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
