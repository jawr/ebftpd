#include <sstream>
#include <iomanip>
#include "stats/util.hpp"

namespace stats { namespace util
{

std::string AutoUnitSpeedString(double speed)
{  
  return AutoUnitString(speed) + "/s";
}

std::string AutoUnitString(double amount)
{
  std::stringstream os;
  os << std::fixed << std::setprecision(2);
  if (amount < 1024.0) os << amount << "B";
  else if (amount < 1024.0 * 1024.0) os << (amount / 1024.0) << "KB";
  else if (amount < 1024.0 * 1024.0 * 1024.0) os << (amount / (1024.0  * 1024.0)) << "MB";
  else os << (amount / (1024.0 * 1024.0 * 1024.0)) << "GB";
  return os.str();
}

std::string HighResSecondsString(const boost::posix_time::time_duration& duration)
{
  std::stringstream os;
  
  if (duration.total_microseconds() < 100000)
  {
    os << std::fixed << std::setprecision(3);
    os << (duration.total_microseconds() / 1000.0 ) << "ms";
  }
  else
  {
    os << std::fixed << std::setprecision(2);
    os << (duration.total_microseconds() / 1000000.0) << "s";
  }
  
  return os.str();
}

} /* util namespace */
} /* stats namespace */
