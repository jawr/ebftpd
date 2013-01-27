#include <sstream>
#include "ftp/counter.hpp"
#include "cfg/get.hpp"

namespace ftp
{

boost::mutex Counter::loggedInMutex;
int Counter::totalLoggedIn = 0;
std::unordered_map<acl::UserID, int> Counter::loggedIn;

boost::mutex Counter::uploadsMutex;
int Counter::totalUploads = 0;
std::unordered_map<acl::UserID, int> Counter::uploads;

boost::mutex Counter::downloadsMutex;
int Counter::totalDownloads = 0;
std::unordered_map<acl::UserID, int> Counter::downloads;

CounterResult Counter::LogIn(acl::UserID uid, int limit, bool kickLogin, bool exempt)
{
  const cfg::Config& config = cfg::Get();
  boost::lock_guard<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  if (count - kickLogin >= limit)
  {
    return CounterResult::PersonalFail;
  }
  int maxUsers = config.MaxUsers().Users();
  if (exempt) maxUsers += config.MaxUsers().ExemptUsers();
  if (totalLoggedIn - kickLogin > maxUsers)
  {
    return CounterResult::GlobalFail;
  }
  ++count;
  ++totalLoggedIn;
  return CounterResult::Okay;
}

void Counter::LogOut(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  if (count > 0) --count;
  if (totalLoggedIn > 0) --totalLoggedIn;
}

CounterResult Counter::StartUpload(acl::UserID uid, int limit, bool exempt)
{
  const cfg::Config& config = cfg::Get();
  boost::lock_guard<boost::mutex> lock(uploadsMutex);
  int& count = uploads[uid];
  if (count >= limit) return CounterResult::PersonalFail;
  if (!exempt && config.SimXfers().MaxUploads() != -1 &&
      totalUploads >= config.SimXfers().MaxUploads())
  {
    return CounterResult::GlobalFail;
  }
  ++count;
  ++totalUploads;
  return CounterResult::Okay;
}

void Counter::StopUpload(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(uploadsMutex);
  int& count = uploads[uid];
  if (count > 0)
  {
    --count;
    --totalUploads;
  }
}

CounterResult Counter::StartDownload(acl::UserID uid, int limit, bool exempt)
{
  const cfg::Config& config = cfg::Get();
  boost::lock_guard<boost::mutex> lock(downloadsMutex);
  int& count = downloads[uid];
  if (count >= limit) return CounterResult::PersonalFail;;
  if (!exempt && config.SimXfers().MaxDownloads() != -1 &&
      totalDownloads >= config.SimXfers().MaxDownloads())
  {
    return CounterResult::GlobalFail;
  }
  ++count;
  ++totalDownloads;
  return CounterResult::Okay;
}

void Counter::StopDownload(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(downloadsMutex);
  int& count = downloads[uid];
  if (count > 0)
  {
    --count;
    --totalDownloads;
  }
}

} /* ftp namespace */
