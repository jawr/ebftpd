#include <sstream>
#include "stats/conversions.hpp"

namespace stats 
{
 
namespace tostring 
{

std::string Kbyte(const Stat& stat)
{
  std::ostringstream os;
  os << std::setprecision(2) << stat.KBytes() / 1024.0 << "KB";
  return os.str();
}

std::string Mbyte(const Stat& stat)
{
  std::ostringstream os;
  os << std::setprecision(2) << stat.KBytes() / 1024.0 / 1024.0 << "MB";
  return os.str();
}

std::string Gbyte(const Stat& stat)
{
  std::ostringstream os;
  os << std::setprecision(2) << stat.KBytes() /1024.0 / 1024.0 / 1024.0 << "GB";
  return os.str();
}


} // end tostring
} // end stats
