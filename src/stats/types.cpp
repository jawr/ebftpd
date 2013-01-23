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

const std::vector< ::stats::Timeframe> timeframes =
{
  ::stats::Timeframe::Day,
  ::stats::Timeframe::Week,
  ::stats::Timeframe::Month,
  ::stats::Timeframe::Alltime,
};

template <> const char* util::EnumStrings<Direction>::values[] =
{
  "up",
  "down",
  ""
};

const std::vector< ::stats::Direction> directions =
{
  ::stats::Direction::Upload,
  ::stats::Direction::Download
};

template <> const char* util::EnumStrings<SortField>::values[] =
{
  "bytes",
  "files",
  "speed"
  ""
};

}
