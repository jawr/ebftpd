#include <sstream>
#include <iomanip>
#include "stats/util.hpp"

namespace stats { namespace util
{
        
std::string AutoUnitSpeedString(double speed)
{  
  std::stringstream os;
  os << std::fixed << std::setprecision(2);
  if (speed < 1024.0) os << speed << "B/S";
  else if (speed < 1024.0 * 1024.0) os << (speed / 1024.0) << "KB/s";
  else if (speed < 1024.0 * 1024.0 * 1024.0) os << (speed / (1024.0  * 1024.0)) << "MB/s";
  else os << (speed / (1024.0 * 1024.0 * 1024.0)) << "GB/s";
  return os.str();
}

} /* util namespace */
} /* stats namespace */
