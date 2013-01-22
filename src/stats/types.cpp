#include "stats/types.hpp"

namespace stats
{

template <> const char* util::EnumStrings<Timeframe>::values[] = 
{
  "day",
  "week",
  "month",
  "year",
  "alltime",
  ""
};

template <> const char* util::EnumStrings<Direction>::values[] =
{
  "up",
  "down",
  ""
};

template <> const char* util::EnumStrings<SortField>::values[] =
{
  "bytes",
  "files",
  "speed"
  ""
};

}
