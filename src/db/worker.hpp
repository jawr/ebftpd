#ifndef __DB_WORKER_HPP
#define __DB_WORKER_HPP

#include <mongo/client/dbclient.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include "util/thread.hpp"
#include "db/types.hpp"
#include "db/taskqueue.hpp"

namespace db
{

class Worker : public util::Thread
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
  void Delete(const std::string& container, mongo::Query& query);
  void EnsureIndex(const std::string& container, const mongo::BSONObj& ob);
  
};

// end
}
#endif
