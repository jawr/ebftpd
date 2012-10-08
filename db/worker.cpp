#include "db/worker.hpp"
#include "db/task.hpp"
#include "db/exception.hpp"
#include "db/taskqueue.hpp"
#include "logger/logger.hpp"

namespace db
{

Worker::Worker(const std::string& host, TaskQueue& queue) : 
  host(host), database("ebftpd."), queue(queue)
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
    //boost::mutex::scoped_lock lock(mtx);
    conn.insert(database + container, obj);
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

void Worker::Update(const std::string& container, mongo::BSONObj& obj,
  mongo::Query& query, bool upsert)
{
  try
  {
    //boost::mutex::scoped_lock lock(mtx);
    conn.update(database + container, query, obj, upsert, false);
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}


void Worker::Get(const std::string& container, const mongo::Query& query, 
  QueryResults& results, int limit)
{
  try
  {
    //boost::mutex::scoped_lock lock(mtx);
    std::unique_ptr<mongo::DBClientCursor> cursor =
      conn.query(database + container, query);
    int i = 0;
    while (cursor->more())
    {
      results.push_back(cursor->nextSafe().copy());
      if (limit != 0 && ++i == limit) break;
    }
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

  
void Worker::EnsureIndex(const std::string& container, const std::string& key)
{
  try
  {
    //boost::mutex::scoped_lock lock(mtx);
    conn.ensureIndex(database + container, BSON(key << 1), true); // unique
    // no need to check for error
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
  }
}

// end
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
    logger::error << e.what() << logger::endl;
    return 1;
  }
  logger::ftpd << "Worker connected" << logger::endl;
  return 0;
}
#endif

