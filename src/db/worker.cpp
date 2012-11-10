#include "db/worker.hpp"
#include "db/task.hpp"
#include "db/exception.hpp"
#include "db/taskqueue.hpp"
#include "logs/logs.hpp"

namespace db
{

Worker::Worker(const std::string& host, 
    const std::string& database,
    TaskQueue& queue,
    const std::string& login,
    const std::string& password) :
  queue(queue)
{
  conn.connect(host);
  if (!login.empty())
  {
    std::string errmsg;
    if (!conn.auth(database, login, password, errmsg))
    {
      throw DBError("Failed to authetnicate with the database.");
    }
  }
}

void Worker::Run()
{
  while (true)
  {
    TaskPtr task = queue.Pop();
    boost::this_thread::disable_interruption noInterrupt;
    task->Execute(conn);
  }
} 

}

#ifdef DB_WORKER_TEST
int main()
{
  try
  {
    db::Worker worker("localhost");
  }
  catch(const db::DBError& e) 
  {
    logs::db << "Worker failed to connect: " << e.what() << logs::endl;
    return 1;
  }
  logs::debug << "Worker connected" << logs::endl;
  return 0;
}
#endif

