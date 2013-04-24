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

#ifndef __SPEEDCOUNTER_HPP
#define __SPEEDCOUNTER_HPP

#include <cassert>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <unordered_map>
#include <functional>
#include <boost/optional.hpp>
#include <string>
#include "acl/types.hpp"

namespace cfg
{
class SpeedLimit;
}

namespace ftp
{

enum class CounterResult : int;

struct SpeedInfo
{
  boost::posix_time::time_duration xfertime;
  long long bytes;
  
  SpeedInfo() :
    xfertime(boost::posix_time::microseconds(0)), bytes(0)
  { }
  
  SpeedInfo(const boost::posix_time::time_duration& xfertime, long long bytes) :
    xfertime(xfertime), bytes(bytes) 
  { }
  
  SpeedInfo& operator+=(const SpeedInfo& rhs)
  {
    xfertime += rhs.xfertime;
    bytes += rhs.bytes;
    return *this;
  }
  
  SpeedInfo& operator-=(const SpeedInfo& rhs)
  {
    xfertime -= rhs.xfertime;
    bytes -= rhs.bytes;
    
    assert(xfertime >= boost::posix_time::microseconds(0));
    assert(bytes >= 0);
    
    return *this;
  }
  
  double Speed() const 
  {
    if (xfertime.total_microseconds() == 0)
      return bytes;
    else
      return bytes / (xfertime.total_microseconds() / 1000000.0);
  }
};

typedef boost::optional<SpeedInfo> SpeedInfoOpt;

class SpeedCounter
{
  std::mutex mutex;
  std::unordered_map<std::string, std::pair<int, SpeedInfo>> speeds;
  std::function<long long(const cfg::SpeedLimit&)> getSpeedLimit;

  SpeedCounter(const std::function<long long(const cfg::SpeedLimit&)>& getSpeedLimit) :
    getSpeedLimit(getSpeedLimit)
  { }
  
  SpeedCounter& operator=(const SpeedCounter&) = delete;
  SpeedCounter& operator=(SpeedCounter&&) = delete;
  SpeedCounter(const SpeedCounter&) = delete;
  SpeedCounter(SpeedCounter&&) = delete;
  
public:

  typedef std::vector<const cfg::SpeedLimit*>& SpeedLimitList;
  
  boost::posix_time::time_duration 
  Update(const SpeedInfoOpt& last, const SpeedInfo& current, const SpeedLimitList& limits);
  void Clear(const SpeedInfoOpt& last, const SpeedLimitList& limits);
  
  friend class Counter;
};

} /* ftp namespace */

#endif
