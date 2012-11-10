#ifndef __FTP_COUNTER_HPP
#define __FTP_COUNTER_HPP

#include <boost/thread/mutex.hpp>
#include <unordered_map>
#include "acl/types.hpp"
#include "util/error.hpp"

namespace ftp
{

class Counter
{
  static boost::mutex loggedInMutex;
  static std::unordered_map<acl::UserID, int> loggedIn;
  static int totalLoggedIn;
  
  static boost::mutex curUploadsMutex;
  static std::unordered_map<acl::UserID, int> curUploads;
  
  static boost::mutex curDownloadsMutex;
  static std::unordered_map<acl::UserID, int> curDownloads;

public:
 static util::Error LogIn(acl::UserID uid, int limit, bool kickLogin, bool exempt);
 static void LogOut(acl::UserID uid);
 
 static bool StartUpload(acl::UserID uid, int limit);
 static void StopUpload(acl::UserID uid);
 
 static bool StartDownload(acl::UserID uid, int limit);
 static void StopDownload(acl::UserID uid);
};

} /* ftp namespace */

#endif
