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
  "upload",
  "download",
  ""
};


}
