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

extern const std::vector< ::stats::Timeframe> timeframes;

enum class Direction : unsigned
{
  Upload,
  Download
};

extern const std::vector< ::stats::Direction> directions;

enum class SortField : unsigned
{
  Bytes,
  Files,
  Speed
};

} /* stats namespace */

namespace util
{

template <> const char* util::EnumStrings<stats::Timeframe>::values[];
template <> const char* util::EnumStrings<stats::SortField>::values[];
template <> const char* util::EnumStrings<stats::Direction>::values[];

}

#endif
