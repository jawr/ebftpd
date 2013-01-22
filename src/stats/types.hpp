#ifndef __STATS_TYPES_HPP
#define __STATS_TYPES_HPP

#include "util/enum.hpp"

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

template <> const char* util::EnumStrings<Timeframe>::values[];

enum class Direction
{
  Upload,
  Download
};

template <> const char* util::EnumStrings<Direction>::values[];

} /* stats namespace */

#endif
