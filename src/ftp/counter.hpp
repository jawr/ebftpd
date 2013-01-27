#ifndef __FTP_COUNTER_HPP
#define __FTP_COUNTER_HPP

#include <boost/thread/mutex.hpp>
#include <unordered_map>
#include "acl/types.hpp"

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

public:
 static CounterResult LogIn(acl::UserID uid, int limit, bool kickLogin, bool exempt);
 static void LogOut(acl::UserID uid);
 
 static CounterResult StartUpload(acl::UserID uid, int limit, bool exempt);
 static void StopUpload(acl::UserID uid);
 
 static CounterResult StartDownload(acl::UserID uid, int limit, bool exempt);
 static void StopDownload(acl::UserID uid);
};

} /* ftp namespace */

#endif
