#include "stats/types.hpp"

namespace util
{
template <> const char* util::EnumStrings<stats::Timeframe>::values[] = 
{
  "day",
  "week",
  "month",
  "year",
  "alltime",
  ""
};

template <> const char* util::EnumStrings<stats::Direction>::values[] =
{
  "up",
  "down",
  ""
};


template <> const char* util::EnumStrings<stats::SortField>::values[] =
{
  "kbytes",
  "files",
  "speed",
  ""
};

}

namespace stats
{

const std::vector<Timeframe> timeframes =
{
  Timeframe::Day,
  Timeframe::Week,
  Timeframe::Month,
  Timeframe::Year,
  Timeframe::Alltime,
};

const std::vector<Direction> directions =
{
  Direction::Upload,
  Direction::Download
};

}
