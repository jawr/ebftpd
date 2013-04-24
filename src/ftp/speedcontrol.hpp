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

#ifndef __FTP_SPEEDCONTROL_HPP
#define __FTP_SPEEDCONTROL_HPP

#include <algorithm>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "ftp/transferstate.hpp"
#include "cfg/setting.hpp"
#include "ftp/error.hpp"
#include "ftp/counter.hpp"
#include "stats/util.hpp"
#include "ftp/client.hpp"
#include "acl/misc.hpp"
#include "ftp/data.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"

namespace ftp
{

class SpeedControl
{
private:
  long long minimumSpeed;
  long long maximumSpeed;
  const TransferState& state;
  std::vector<const cfg::SpeedLimit*> globalLimits;
  SpeedCounter& globalCounter;
  boost::posix_time::ptime lastMinimumOk;
  SpeedInfoOpt lastSpeedInfo;
  
  static const int minimumSpeedKickTime = 5;
  
  inline void CheckMinimum(double speed)
  {
    auto now = boost::posix_time::microsec_clock::local_time();
    if (speed > minimumSpeed)
    {
      lastMinimumOk = now;
    }
    else
    if ((now - lastMinimumOk).total_seconds() > minimumSpeedKickTime)
    {
      throw ftp::MinimumSpeedError(minimumSpeed, speed);
    }
  }

protected:
  SpeedControl(int minimumSpeed, int maximumSpeed, 
                  const TransferState& state, 
                  std::vector<const cfg::SpeedLimit*>&& globalLimits,
                  SpeedCounter& globalCounter) :
    minimumSpeed(minimumSpeed),
    maximumSpeed(maximumSpeed),
    state(state),
    globalLimits(globalLimits),
    globalCounter(globalCounter),
    lastMinimumOk(boost::posix_time::microsec_clock::local_time())
  {
  }
  
public:
  inline void Apply()
  {
    if (minimumSpeed == 0 && maximumSpeed == 0 && globalLimits.empty()) return;

    auto speedInfo = ftp::SpeedInfo(state.Duration(), state.Bytes());
    
    if (minimumSpeed > 0)
    {
      CheckMinimum(speedInfo.Speed() / 1024);
    }
    
    boost::posix_time::time_duration sleepTime(boost::posix_time::microseconds(0));
    
    if (maximumSpeed > 0)
    {
      sleepTime = stats::SpeedLimitSleep(speedInfo.xfertime, speedInfo.bytes, 
                                         maximumSpeed * 1024);
    }
    
    if (!globalLimits.empty())
    {
      sleepTime = std::max(sleepTime, globalCounter.Update(lastSpeedInfo, speedInfo, globalLimits));
    }
    
    boost::this_thread::sleep(sleepTime);
    lastSpeedInfo = speedInfo;
  }
  
  virtual ~SpeedControl()
  {
    globalCounter.Clear(lastSpeedInfo, globalLimits);
  }
};

class UploadSpeedControl : public SpeedControl
{
public:
  UploadSpeedControl(const ftp::Client& client, const fs::VirtualPath& path) :
    SpeedControl(acl::speed::UploadMinimum(client.User(), path),
                 client.User().MaxUpSpeed(),
                 client.Data().State(),
                 acl::speed::UploadMaximum(client.User(), path),
                 Counter::UploadSpeeds())
  {
  }
};

class DownloadSpeedControl : public SpeedControl
{
public:
  DownloadSpeedControl(const ftp::Client& client, const fs::VirtualPath& path) :
    SpeedControl(acl::speed::DownloadMinimum(client.User(), path),
                 client.User().MaxDownSpeed(),
                 client.Data().State(),
                 acl::speed::DownloadMaximum(client.User(), path),
                 Counter::DownloadSpeeds())
  {
  }
};

} /* ftp namespace */

#endif
