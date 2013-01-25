#ifndef __ACL_REPLICATOR_HPP
#define __ACL_REPLICATOR_HPP

#include <atomic>
#include <future>
#include <boost/thread/mutex.hpp>
#include "acl/replicable.hpp"
#include "logs/logs.hpp"

namespace acl
{

class Replicator
{
  std::vector<Replicable*> caches;
  int interval;
  
  std::atomic<bool> enabled;
  boost::mutex busy;
  typedef boost::lock_guard<boost::mutex> BusyGuard;
  
  static std::shared_ptr<Replicator> instance;
  static const int maximumRetries = 20;
  
  Replicator(int interval);
  
  void Run(const std::shared_ptr<BusyGuard>& lock);
  
  void InnerCancel()
  {
    enabled = false;
    boost::lock_guard<boost::mutex> lock(busy);
  }

  void InnerReplicate();
  void ResetTimer();
  
public:

  static void Initialise(int interval)
  {
    if (interval > 0) instance.reset(new Replicator(interval));
  }

  static std::shared_ptr<Replicator> Get() { return instance; }
  
  static void Cancel()
  {
    if (instance.get()) instance->InnerCancel();
  }
  
  static void Register(Replicable* cache)
  {
    if (instance.get()) instance->caches.push_back(cache);
  }
  
  static void Replicate()
  {
    if (instance.get()) instance->InnerReplicate();
  }
};

} /* acl namespace */

#endif
