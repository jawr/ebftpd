#ifndef __FTP_COUNTER_HPP
#define __FTP_COUNTER_HPP

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <unordered_map>
#include <vector>
#include <utility>
#include "acl/types.hpp"

namespace cfg { namespace setting
{
class SpeedLimit;
}
}

namespace ftp
{

enum class CounterResult
{
  PersonalFail,
  GlobalFail,
  Okay
};

class Counter
{
  static boost::mutex loggedInMutex;
  static int totalLoggedIn;
  static std::unordered_map<acl::UserID, int> loggedIn;
  
  static boost::mutex uploadsMutex;
  static int totalUploads;
  static std::unordered_map<acl::UserID, int> uploads;
  
  static boost::mutex downloadsMutex;
  static int totalDownloads;
  static std::unordered_map<acl::UserID, int> downloads;
  
  static boost::mutex uploadSpeedMutex;
  static std::unordered_map<std::string, std::pair<int, double>> uploadSpeeds;
  
  static boost::mutex downloadSpeedMutex;
  static std::unordered_map<std::string, std::pair<int, double>> downloadSpeeds;

public:
 static CounterResult LogIn(acl::UserID uid, int limit, bool kickLogin, bool exempt);
 static void LogOut(acl::UserID uid);
 
 static CounterResult StartUpload(acl::UserID uid, int limit, bool exempt);
 static void StopUpload(acl::UserID uid);
 
 static CounterResult StartDownload(acl::UserID uid, int limit, bool exempt);
 static void StopDownload(acl::UserID uid);
 
 typedef std::vector<const cfg::setting::SpeedLimit*>& SpeedLimitList;

 static boost::posix_time::time_duration 
        UploadSpeedLimitSleep(double lastSpeed, double curSpeed, const SpeedLimitList& limits);
 static void UploadSpeedLimitClear(double lastSpeed, const SpeedLimitList& limits);
          
 static boost::posix_time::time_duration 
        DownloadSpeedLimitSleep(double lastSpeed, double curSpeed, const SpeedLimitList& limits);
 static void DownloadSpeedLimitClear(double lastSpeed, const SpeedLimitList& limits);
};

} /* ftp namespace */

#endif
