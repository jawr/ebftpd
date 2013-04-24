//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __STATS_TYPES_HPP
#define __STATS_TYPES_HPP

#include <vector>
#include "util/enumstrings.hpp"

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
  KBytes,
  Files,
  Speed
};

} /* stats namespace */

namespace util
{

template <> const char* EnumStrings<stats::Timeframe>::values[];
template <> const char* EnumStrings<stats::SortField>::values[];
template <> const char* EnumStrings<stats::Direction>::values[];

}

#endif
