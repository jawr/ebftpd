#ifndef __DB_WORKER_HPP
#define __DB_WORKER_HPP
#include "util/thread.hpp"
#include "db/types.hpp"
#include "db/taskqueue.hpp"
#include <mongo/client/dbclient.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

namespace db
{

class Worker : public util::ThreadConsumer
{
  mongo::DBClientConnection conn;
  std::string host;
  std::string database;

  TaskQueue& queue;

  virtual void Run();

public:
  Worker(const std::string& host, TaskQueue& queue);

  void Insert(const std::string& container, const mongo::BSONObj& obj);
  void Get(const std::string& container, const mongo::Query& query, 
    QueryResults& results, int limit=0);
  void Update(const std::string&, mongo::BSONObj&, 
    mongo::Query& query, bool upsert);
  void EnsureIndex(const std::string& container, const std::string& key);
  
};

// end
}
#endif
