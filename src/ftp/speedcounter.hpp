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

namespace cfg { namespace setting
{
class SpeedLimit;
}
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
      return bytes / xfertime.total_microseconds() / 1000000.0;
  }
};

typedef boost::optional<SpeedInfo> SpeedInfoOpt;

class SpeedCounter
{
  std::mutex mutex;
  std::unordered_map<std::string, std::pair<int, SpeedInfo>> speeds;
  std::function<int(const cfg::setting::SpeedLimit&)> getSpeedLimit;

  SpeedCounter(const std::function<int(const cfg::setting::SpeedLimit&)>& getSpeedLimit) :
    getSpeedLimit(getSpeedLimit)
  { }
  
  SpeedCounter& operator=(const SpeedCounter&) = delete;
  SpeedCounter& operator=(SpeedCounter&&) = delete;
  SpeedCounter(const SpeedCounter&) = delete;
  SpeedCounter(SpeedCounter&&) = delete;
  
public:

  typedef std::vector<const cfg::setting::SpeedLimit*>& SpeedLimitList;
  
  boost::posix_time::time_duration 
  Update(const SpeedInfoOpt& last, const SpeedInfo& current, const SpeedLimitList& limits);
  void Clear(const SpeedInfoOpt& last, const SpeedLimitList& limits);
  
  friend class Counter;
};

} /* ftp namespace */

#endif
