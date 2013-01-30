#include <sstream>
#include "ftp/counter.hpp"
#include "cfg/get.hpp"
#include "stats/util.hpp"

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

boost::mutex Counter::uploadSpeedMutex;
std::unordered_map<std::string, std::pair<int, double>> Counter::uploadSpeeds;

boost::mutex Counter::downloadSpeedMutex;
std::unordered_map<std::string, std::pair<int, double>> Counter::downloadSpeeds;

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

// blatant ugly copy / paste -- should be refactored into a speed limiter class

boost::posix_time::time_duration 
Counter::UploadSpeedLimitSleep(double lastSpeed, double curSpeed, const SpeedLimitList& limits)
{
  boost::posix_time::time_duration sleep = boost::posix_time::microseconds(0);
  boost::lock_guard<boost::mutex> lock(uploadSpeedMutex);
  for (const auto& limit : limits)
  {
    auto it = uploadSpeeds.find(limit->Path().ToString());
    if (it == uploadSpeeds.end())
    {
      auto result = uploadSpeeds.insert(std::make_pair(
            limit->Path().ToString(), std::make_pair(1, curSpeed)));
      it = result.first;
    }
    else
    {
      if (lastSpeed > 0) it->second.second -= lastSpeed;
      else ++it->second.first;
      it->second.second += curSpeed;
    }
    sleep = std::max(sleep, stats::SpeedLimitSleep(it->second.second, limit->UlLimit()));
  }
  return sleep;
}

void Counter::UploadSpeedLimitClear(double lastSpeed, const SpeedLimitList& limits)
{
  boost::lock_guard<boost::mutex> lock(uploadSpeedMutex);
  for (const auto& limit : limits)
  {
    auto it = uploadSpeeds.find(limit->Path().ToString());
    if (it != uploadSpeeds.end())
    {
      if (--it->second.first <= 0)
      {
        uploadSpeeds.erase(it);
      }
      else
        it->second.second -= lastSpeed;
    }
  }
}
        
boost::posix_time::time_duration 
Counter::DownloadSpeedLimitSleep(double lastSpeed, double curSpeed, const SpeedLimitList& limits)
{
  boost::posix_time::time_duration sleep = boost::posix_time::microseconds(0);
  boost::lock_guard<boost::mutex> lock(downloadSpeedMutex);
  for (const auto& limit : limits)
  {
    auto it = downloadSpeeds.find(limit->Path().ToString());
    if (it == downloadSpeeds.end())
    {
      auto result = downloadSpeeds.insert(std::make_pair(
            limit->Path().ToString(), std::make_pair(1, curSpeed)));
      it = result.first;
    }
    else
    {
      if (lastSpeed > 0) it->second.second -= lastSpeed;
      else ++it->second.first;
      it->second.second += curSpeed;
    }
    sleep = std::max(sleep, stats::SpeedLimitSleep(it->second.second, limit->DlLimit()));
  }
  return sleep;
}

void Counter::DownloadSpeedLimitClear(double lastSpeed, const SpeedLimitList& limits)
{
  boost::lock_guard<boost::mutex> lock(downloadSpeedMutex);
  for (const auto& limit : limits)
  {
    auto it = downloadSpeeds.find(limit->Path().ToString());
    if (it != downloadSpeeds.end())
    {
      if (--it->second.first <= 0)
      {
        downloadSpeeds.erase(it);
      }
      else
        it->second.second -= lastSpeed;
    }
  }
}

} /* ftp namespace */
