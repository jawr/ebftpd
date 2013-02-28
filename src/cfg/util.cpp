#include <boost/lexical_cast.hpp>
#include "cfg/util.hpp"
#include "util/string.hpp"

namespace cfg
{

bool YesNoToBoolean(std::string s)
{
  util::ToLower(s);
  if (s == "yes") return true;
  if (s == "no") return false;
  throw std::bad_cast();
}

long long ParseSize(std::string s)
{
  if (s.empty()) throw std::bad_cast();
  
  std::string unit = "K";
  if (std::isalpha(s.back()))
  {
    unit.assign(s.end() - 1, s.end());
    s.erase(s.end() - 1, s.end());
    if (s.empty()) throw std::bad_cast();
    util::ToUpper(unit);
  }
  
  long long kBytes;
  try
  {
    kBytes = boost::lexical_cast<long long>(s);
    if (kBytes < 0) throw std::bad_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw std::bad_cast();
  }
  
  if (unit == "M") kBytes *= 1024;
  else if (unit == "G") kBytes *= 1024 * 1024;
  else if (unit != "K") throw std::bad_cast();
  
  return kBytes;
}

} /* cfg namespace */
