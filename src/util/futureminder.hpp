#ifndef __UTIL_FUTUREMINDER_HPP
#define __UTIL_FUTUREMINDER_HPP

#include <memory>
#include <algorithm>
#include <future>
#include <chrono>
#include <boost/thread/once.hpp>
#include <boost/thread/future.hpp>

namespace util
{

class FutureMinder
{
  boost::mutex mutex;
  boost::once_flag initOnce;
  std::vector<std::future<void>> futures;

  static bool IsReady(std::future<void>& future)
  {
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 6)
    return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
#else
    return future.wait_for(std::chrono::seconds(0));
#endif
  }

  void EraseReady()
  {
    futures.erase(std::remove_if(futures.begin(), futures.end(), &IsReady), futures.end());
  }
  
  FutureMinder& operator=(const FutureMinder&) = delete;
  FutureMinder(const FutureMinder&) = delete;
  
public:
  FutureMinder() = default;
  
  virtual ~FutureMinder()
  {
    try
    {
      while (!futures.empty())
      {
        EraseReady();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
      }
    }
    catch (...)
    {
    }
  }

  void Assign(std::future<void>&& future)
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    futures.emplace_back(std::move(future));
    EraseReady();
  }
};

} /* util namespace */

#endif
