#include "db/worker.hpp"
#include "db/task.hpp"
#include "db/exception.hpp"
#include "db/taskqueue.hpp"
#include "logs/logs.hpp"

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

void Worker::Delete(const std::string& container, mongo::Query& query)
{
  try
  {
    conn.remove(database + container, query);
    const std::string& err = conn.getLastError();
    if (err.size() > 0) throw DBError(err);
  }
  catch (const mongo::DBException& e)
  {
    throw DBError(e.what());
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
  results.clear();
  results.reserve(limit);
  
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

  
void Worker::EnsureIndex(const std::string& container, const mongo::BSONObj& obj)
{
  try
  {
    //boost::mutex::scoped_lock lock(mtx);
    conn.ensureIndex(database + container, obj, true); // unique
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
    logs::db << "Worker failed to connect: " << e.what() << logs::endl;
    return 1;
  }
  logs::debug << "Worker connected" << logs::endl;
  return 0;
}
#endif

