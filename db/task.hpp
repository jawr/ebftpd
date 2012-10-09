#ifndef __DB_TASK_HPP
#define __DB_TASK_HPP
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include "db/worker.hpp"

namespace db
{

class Task
{
public:
  virtual ~Task() {};
  virtual void Execute(Worker& worker) = 0;
};

class Select : public Task
{
  const std::string& container;
  const mongo::Query& query;
  QueryResults& results;
  boost::unique_future<bool>& future;
public:
  Select(const std::string& container, const mongo::Query& query,
    QueryResults& results, boost::unique_future<bool>& future) : 
      container(container), query(query), results(results), future(future) {};
  virtual void Execute(Worker& worker);
};


// end
}
#endif
