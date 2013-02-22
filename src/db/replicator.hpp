#ifndef __DB_REPLICATOR_HPP
#define __DB_REPLICATOR_HPP

#include <atomic>
#include <future>
#include <memory>
#include <boost/thread/thread.hpp>
#include <mutex>
#include "db/replicable.hpp"

namespace db
{

class Replicator
{
  boost::thread thread;
  std::vector<std::shared_ptr<Replicable>> caches;

  static std::shared_ptr<Replicator> instance;
  static const int maximumRetries = 20;
  
  Replicator() = default;
  
  void Run();  
  void LogFailed(const std::list<std::shared_ptr<Replicable>>& failed);
  void Replicate(const mongo::BSONObj& entry);
  void Populate();
  
public:
  void Start();
  void Stop();
  
  bool Register(const std::shared_ptr<Replicable>& cache);

  static Replicator& Get()
  {
    if (!instance) instance.reset(new Replicator());
    return *instance;
  }  
};

} /* db namespace */

#endif
