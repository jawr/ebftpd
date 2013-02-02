#include "db/worker.hpp"
#include "db/task.hpp"
#include "db/error.hpp"
#include "db/taskqueue.hpp"

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
    auto task = queue.Pop();
    boost::this_thread::disable_interruption noInterrupt;
    task->Execute(conn);
  }
} 

}
