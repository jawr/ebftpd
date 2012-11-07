#include "ftp/counter.hpp"

#include <iostream>

namespace ftp
{

boost::mutex Counter::loggedInMutex;
std::unordered_map<acl::UserID, int> Counter::loggedIn;

boost::mutex Counter::curUploadsMutex;
std::unordered_map<acl::UserID, int> Counter::curUploads;

boost::mutex Counter::curDownloadsMutex;
std::unordered_map<acl::UserID, int> Counter::curDownloads;

bool Counter::LogIn(acl::UserID uid, int limit)
{
  boost::lock_guard<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  std::cout << count << " " << limit << std::endl;
  if (count >= limit) return false;
  ++count;
  return true;
}

void Counter::LogOut(acl::UserID uid)
{
  boost::lock_guard<boost::mutex> lock(loggedInMutex);
  int& count = loggedIn[uid];
  if (count > 0) --count;
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
