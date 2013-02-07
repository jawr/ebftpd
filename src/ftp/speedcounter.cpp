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
  boost::lock_guard<boost::mutex> lock(mutex);
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
                                                   getSpeedLimit(*limit)));
  }
  return sleep;
}

void SpeedCounter::Clear(const SpeedInfoOpt& last, const SpeedLimitList& limits)
{
  if (!last) return;
  
  boost::lock_guard<boost::mutex> lock(mutex);
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
