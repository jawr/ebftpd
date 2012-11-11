#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include "stats/types.hpp"

namespace stats
{
namespace
{
const std::string timeframeStrings[] =
{
  "day",
  "week",
  "month",
  "year",
  "alltime"
};
}

std::string TimeframeToString(Timeframe tf)
{
  unsigned index = static_cast<unsigned>(tf);
  assert(index < sizeof(timeframeStrings) / sizeof(std::string));
  return timeframeStrings[index];
}

Timeframe TimeframeFromString(std::string s)
{
  boost::to_lower(s);
  auto begin = std::begin(timeframeStrings);
  auto end = std::end(timeframeStrings);
  auto it = std::find(begin, end, s);
  if (it == end) throw std::out_of_range("Invalid timeframe string");
  return static_cast<Timeframe>(std::distance(begin, it));
}

} /* stats namespace */