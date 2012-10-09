#include "db/pool.hpp"
#include "db/exception.hpp"
#include "db/get.hpp"
#include "logger/logger.hpp"
#include "cfg/get.hpp"
#include <tr1/memory>
#include "db/task.hpp"
namespace db
{

void Pool::Run()
{
  logger::ftpd << "db::Pool started" << logger::endl;

  for (int i = 0; i < 5; ++i)
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
  std::tr1::shared_ptr<db::Pool> pool(new db::Pool());
  pool->Start();
  db::UpdateShared(pool);
  db::Pool& poolRef = db::Get();
  mongo::Query query;
  db::QueryResults results;
  boost::unique_future<bool> future;
  db::TaskPtr task(new db::Select("ebftpd.users", query, results, future));
  poolRef.Queue(task);
  pool->Join();
  
}
#endif 
