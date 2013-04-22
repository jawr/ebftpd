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

#include <algorithm>
#include "ftp/speedcounter.hpp"
#include "stats/util.hpp"
#include "cfg/setting.hpp"

namespace ftp
{

boost::posix_time::time_duration 
SpeedCounter::Update(const SpeedInfoOpt& last, const SpeedInfo& current, const SpeedLimitList& limits)
{
  boost::posix_time::time_duration sleep = boost::posix_time::microseconds(0);
  std::lock_guard<std::mutex> lock(mutex);
  for (const auto& limit : limits)
  {
    auto it = speeds.find(limit->Path());
    if (it == speeds.end())
    {
      auto result = speeds.insert(std::make_pair(
            limit->Path(), std::make_pair(1, current)));
      it = result.first;
    }
    else
    {
      if (last) it->second.second -= *last;
      else ++it->second.first;
      it->second.second += current;
    }

    sleep = std::max(sleep, stats::SpeedLimitSleep(it->second.second.xfertime / it->second.first, 
                                                   it->second.second.bytes, 
                                                   getSpeedLimit(*limit) * 1024));
  }
  return sleep;
}

void SpeedCounter::Clear(const SpeedInfoOpt& last, const SpeedLimitList& limits)
{
  if (!last) return;
  
  std::lock_guard<std::mutex> lock(mutex);
  for (const auto& limit : limits)
  {
    auto it = speeds.find(limit->Path());
    if (it != speeds.end())
    {
      if (--it->second.first <= 0)
      {
        speeds.erase(it);
      }
      else
        it->second.second -= *last;
    }
  }
}

} /* ftp namespace */
