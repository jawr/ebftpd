#include "db/pool.hpp"
#include "db/exception.hpp"
#include "logger/logger.hpp"
#include "cfg/get.hpp"
#include <tr1/memory>
#include "db/task.hpp"
namespace db
{

Pool Pool::instance;

void Pool::Run()
{
  logger::ftpd << "db::Pool started" << logger::endl;

  for (int i = 0; i < 8; ++i)
  {
    try
    {
      std::unique_ptr<Worker> worker(new Worker("localhost", queue));
      worker->Start();
      workers.push_back(worker.release());
    }
    catch (const DBError& e)
    {
       throw e;
    }
  }

  while (true)
  {
    TaskQueue::size_type size = queue.WaitChanged();
  }
    
}

// end
}

#ifdef DB_POOL_TEST
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>

int main()
{
  db::Pool::StartThread();
  for (int i = 0; i < 10; ++i)
  {
    mongo::Query query;
    db::QueryResults results;
    boost::unique_future<bool> future;
    db::TaskPtr task(new db::Select("users", query, results, future));
    db::Pool::Queue(task);
  }
  db::Pool::JoinThread();
  
}
#endif 
