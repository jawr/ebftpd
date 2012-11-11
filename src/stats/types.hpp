#ifndef __STATS_TYPES_HPP
#define __STATS_TYPES_HPP

namespace stats
{

enum class Timeframe
{
  Day,
  Week,
  Month,
  Year,
  Alltime
};

std::string TimeframeToString(Timeframe tf);
Timeframe TimeframeFromString(std::string& s);

} /* stats namespace */

#endif
