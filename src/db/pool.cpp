#include <memory>
#include "db/pool.hpp"
#include "db/error.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "db/task.hpp"

namespace db
{

Pool Pool::instance;

Pool::Pool() :
  maxWorkers(boost::thread::hardware_concurrency()),
  started(false)
{
  if (!maxWorkers) maxWorkers = defaultMaxWorkers;
}

void Pool::LaunchWorker()
{
  const cfg::setting::Database& dbConfig = cfg::Get().Database();
  std::ostringstream host;
  host << dbConfig.Address() << ":" << dbConfig.Port();
  
  try
  {
    std::unique_ptr<Worker> worker(new Worker(host.str(), dbConfig.Name(), queue, 
        dbConfig.Login(), dbConfig.Password()));
    worker->Start();
    workers.push_back(worker.release());
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Failed to launch worker for db thread pool: " << e.what() << logs::endl;
  }
  catch (const DBError& e)
  {
    logs::db << "Failed to launch worker for db thread pool: " << e.Message() << logs::endl;
  }
}

void Pool::KillWorker()
{
  workers.back().Stop(true);
  workers.pop_back();
}

void Pool::Finalise()
{
  queue.WaitEmpty();
  while (!workers.empty()) KillWorker();
}

void Pool::HandleWorkers(size_t queueSize)
{
  if (!queueSize)
  {
    while (workers.size() > 1) KillWorker();
  }
  else if (queueSize > maxQueueContention &&
           workers.size() < maxWorkers)
  {
    LaunchWorker();
  }
}

void Pool::Run()
{
  TaskQueue::size_type queueSize = 0;
  while (true)
  {
    queueSize = queue.WaitChanged(queueSize);

    {
      boost::this_thread::disable_interruption noInterrupt;
      HandleWorkers(queueSize);
    }
  }    
}

void Pool::StartThread()
{
  logs::debug << "Starting mongodb connection thread pool.." << logs::endl;
  instance.LaunchWorker();
  if (instance.workers.empty()) throw DBError("Failed to start database thread pool.");
  instance.Start();
}

void Pool::StopThread()
{
  logs::debug << "Stopping mongodb connection thread pool.." << logs::endl;
  instance.Stop(true);
  logs::debug << "Finalising last database transactions.." << logs::endl;
  instance.Finalise();
}

// end
}
