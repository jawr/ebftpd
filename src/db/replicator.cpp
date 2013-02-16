#include <list>
#include <csignal>
#include "db/replicator.hpp"
#include "logs/logs.hpp"

namespace db
{

std::shared_ptr<Replicator> Replicator::instance;

Replicator::Replicator(int interval) :
  interval(interval),
  enabled(true)
{
  ResetTimer();
}

void Replicator::ResetTimer()
{
  if (enabled) alarm(interval);
}

void Replicator::LogFailed(const std::list<Replicable*>& failed)
{
  logs::db << "Exceeded maximum retries while replicating caches: ";
  for (auto it = failed.begin(); it != failed.end(); ++it)
  {
    if (it != failed.begin()) logs::db << ", ";
    logs::db << (*it)->Name();
  }
  logs::db << logs::endl;
}

void Replicator::Run(const std::shared_ptr<BusyGuard>& lock)
{
  std::list<Replicable*> notDone(caches.begin(), caches.end());

  for (int i = 0; i < maximumRetries && !notDone.empty() && enabled; ++i)
  {
    auto it = notDone.begin();
    while (it != notDone.end() && enabled)
    {
      if ((*it)->Replicate()) notDone.erase(it++);
      else ++it;
    }
  }
  
  if (!notDone.empty()) LogFailed(notDone);
  ResetTimer();
  
  (void) lock;
}

void Replicator::InnerReplicate()
{
  if (!busy.try_lock())
  {
    logs::db << "Skipping replication as replication already in progress, "
             << "check your cache_replicate interval." << logs::endl;
    return;
  }

  auto lock = std::make_shared<BusyGuard>(busy, boost::adopt_lock); 
  if (!enabled) return;
  
  std::async(std::launch::async, &Replicator::Run, this, lock);
}

} /* db namespace */
