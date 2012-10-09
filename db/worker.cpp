#include "db/worker.hpp"
#include "db/task.hpp"
#include "db/exception.hpp"
#include "db/taskqueue.hpp"
namespace db
{
Worker::Worker(const std::string& host, TaskQueue& queue) : 
  host(host), queue(queue), database("ebftpd")
{
  try
  {
    conn.connect(host);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

void Worker::Run()
{
  while (true)
  {
    TaskPtr task = queue.Pop();
    task->Execute(*this);
  }
} 

void Worker::Insert(const std::string& container, const mongo::BSONObj& obj)
{
  try
  {
    boost::mutex::scoped_lock lock(mtx);
    conn.insert(database + container, obj);
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

void Worker::Get(const std::string& container, const mongo::Query& query, 
  QueryResults& results)
{
  try
  {
    boost::mutex::scoped_lock lock(mtx);
    std::unique_ptr<mongo::DBClientCursor> cursor =
      conn.query(database + container, query);
    while (cursor->more())
      results.push_back(cursor->nextSafe().copy());
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

// end
}

#ifdef DB_WORKER_TEST
#include "logger/logger.hpp"
int main()
{
  try
  {
    db::Worker worker("localhost");
  }
  catch(const db::DBError& e) 
  {
    logger::error << e.what() << logger::endl;
    return 1;
  }
  logger::ftpd << "Worker connected" << logger::endl;
  return 0;
}
#endif

