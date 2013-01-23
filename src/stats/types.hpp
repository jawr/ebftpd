#ifndef __STATS_TYPES_HPP
#define __STATS_TYPES_HPP

#include <vector>
#include "util/enum.hpp"

namespace stats
{

enum class Timeframe : unsigned
{
  Day,
  Week,
  Month,
  Year,
  Alltime
};

template <> const char* util::EnumStrings<Timeframe>::values[];

extern const std::vector< ::stats::Timeframe> timeframes;

enum class Direction : unsigned
{
  Upload,
  Download
};

extern const std::vector< ::stats::Direction> directions;

template <> const char* util::EnumStrings<Direction>::values[];

enum class SortField : unsigned
{
  Bytes,
  Files,
  Speed
};

template <> const char* util::EnumStrings<SortField>::values[];


} /* stats namespace */

#endif
