#ifndef __DB_TASK_HPP
#define __DB_TASK_HPP
#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include "db/worker.hpp"
#include <iostream>

namespace db
{

class Task
{
public:
  virtual ~Task() {};
  virtual void Execute(Worker& worker) = 0;
};

class EnsureIndex : public Task
{
  std::string container;
  std::string key;
public:
  EnsureIndex(const std::string& container, const std::string& key) :
      container(container), key(key) {};
  virtual void Execute(Worker& worker);
};

class Select : public Task
{
  std::string container;
  const mongo::Query& query;
  QueryResults& results;
  boost::unique_future<bool>& future;
  boost::promise<bool> promise;
  int limit;
public:
  Select(const std::string& container, const mongo::Query& query,
    QueryResults& results, boost::unique_future<bool>& future, int limit = 0) : 
      container(container), query(query), results(results), future(future),
      promise(), limit(limit) { future = promise.get_future(); }

  virtual void Execute(Worker& worker);
};

class Update : public Task
{
protected:
  std::string container;
  mongo::BSONObj obj;
  mongo::Query query;
  bool upsert;
public:
  Update(const std::string& container, const mongo::Query& query, 
    const mongo::BSONObj& obj, bool upsert=false) :
    container(container), obj(obj), query(query), upsert(upsert) {};
  virtual void Execute(Worker& worker);
};


// end
}
#endif
