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

#include <sstream>
#include "util/timepair.hpp"
#include "util/string.hpp"

namespace util
{

std::string FormatDuration(const TimePair& timePair)
{
  static const time_t secondsInDay = 24 * 60 * 60;
  static const time_t secondsInHour = 60 * 60;
  static const time_t secondsInMinute = 60;
  
  auto totalSeconds = timePair.Seconds();

  unsigned days = totalSeconds / secondsInDay;
  totalSeconds %= secondsInDay;
  
  unsigned hours = totalSeconds / secondsInHour;
  totalSeconds %= secondsInHour;
  
  unsigned minutes = totalSeconds / secondsInMinute;
  totalSeconds %= secondsInMinute;
  
  unsigned milliseconds = timePair.Microseconds() / 1000;
  
  std::ostringstream format;
  if (days) format << days << 'd';
  
  if (hours || !format.str().empty())
  {
    format << ' ' << hours << 'h';
  }
  
  if (minutes || !format.str().empty())
  {
    format << ' ' << minutes << 'm';
  }
  
  format << ' ' << totalSeconds;
  if (milliseconds) format << "." << milliseconds;
  format << 's';
  
  return util::TrimCopy(format.str());
}

} /* util namespace */
