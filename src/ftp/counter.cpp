#include <sstream>
#include "ftp/counter.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"

namespace ftp
{

boost::mutex Counter::loggedInMutex;
std::unordered_map<acl::UserID, int> Counter::loggedIn;
int Counter::totalLoggedIn = 0;

boost::mutex Counter::curUploadsMutex;
std::unordered_map<acl::UserID, int> Counter::curUploads;

boost::mutex Counter::curDownloadsMutex;
std::unordered_map<acl::UserID, int> Counter::curDownloads;

util::Error Counter::LogIn(acl::UserID uid, int limit, bool kickLogin, bool exempt)
{
  const cfg::Config& config = cfg::Get();
  boost::unique_lock<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  if (count - kickLogin >= limit)
  {
    lock.release()->unlock();
    std::ostringstream os;
    os << "You have reached your maximum number of " 
       << limit << " login(s).";
    return util::Error::Failure(os.str());
  }
  int maxUsers = config.MaxUsers().Users();
  if (exempt) maxUsers += config.MaxUsers().ExemptUsers();
  logs::debug << "maxusers: " << maxUsers << " tl: " << totalLoggedIn << logs::endl;
  if (totalLoggedIn - kickLogin > maxUsers)
  {
    lock.release()->unlock();
    return util::Error::Failure(
        "Site has reached it's maximuim number of logged in users.");
  }
  ++count;
  ++totalLoggedIn;
  return util::Error::Success();
}

void Counter::LogOut(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  if (count > 0) --count;
  if (totalLoggedIn > 0) --totalLoggedIn;
}

bool Counter::StartUpload(acl::UserID uid, int limit)
{
  boost::lock_guard<boost::mutex> lock(curUploadsMutex);
  int& count = curUploads[uid];
  if (count >= limit) return false;
  ++count;
  return true;
}

void Counter::StopUpload(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(curUploadsMutex);
  int& count = curUploads[uid];
  if (count > 0) --count;
}

bool Counter::StartDownload(acl::UserID uid, int limit)
{
  boost::lock_guard<boost::mutex> lock(curDownloadsMutex);
  int& count = curDownloads[uid];
  if (count >= limit) return false;
  ++count;
  return true;
}

void Counter::StopDownload(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(curDownloadsMutex);
  int& count = curDownloads[uid];
  if (count > 0) --count;
}

} /* ftp namespace */
