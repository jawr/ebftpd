#include <sstream>
#include <boost/algorithm/string/trim.hpp>
#include "util/timepair.hpp"

namespace util
{

std::string FormatDuration(const TimePair& timePair)
{
  static const time_t secondsInDay = 24 * 60 * 60;
  static const time_t secondsInHour = 60 * 60;
  static const time_t secondsInMinute = 60;
  
  auto totalSeconds = timePair.Seconds();

  unsigned days = totalSeconds / secondsInDay;
  totalSeconds %= secondsInDay;
  
  unsigned hours = totalSeconds / secondsInHour;
  totalSeconds %= secondsInHour;
  
  unsigned minutes = totalSeconds / secondsInMinute;
  totalSeconds %= secondsInMinute;
  
  unsigned milliseconds = timePair.Microseconds() / 1000;
  
  std::ostringstream format;
  if (days) format << days << 'd';
  
  if (hours || !format.str().empty())
  {
    format << ' ' << hours << 'h';
  }
  
  if (minutes || !format.str().empty())
  {
    format << ' ' << minutes << 'm';
  }
  
  format << ' ' << totalSeconds;
  if (milliseconds) format << "." << milliseconds;
  format << 's';
  
  return boost::trim_copy(format.str());
}

} /* util namespace */
