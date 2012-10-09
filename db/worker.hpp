#ifndef __DB_WORKER_HPP
#define __DB_WORKER_HPP
#include "util/thread.hpp"
#include "db/taskqueue.hpp"
#include <mongo/client/dbclient.h>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
namespace db
{
typedef std::vector<mongo::BSONObj> QueryResults;

class Worker : public util::ThreadConsumer
{
  mongo::DBClientConnection conn;
  std::string host;

  TaskQueue& queue;

  virtual void Run();

public:
  Worker(const std::string& host, TaskQueue& queue);

  void Insert(const std::string& container, const mongo::BSONObj& obj);
  void Get(const std::string& container, const mongo::Query& query, 
    QueryResults& results);
  
};

// end
}
#endif
