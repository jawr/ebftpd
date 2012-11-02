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
  mongo::BSONObj obj;
public:
  EnsureIndex(const std::string& container, const mongo::BSONObj& obj) :
      container(container), obj(obj) {};
  void Execute(Worker& worker);
};

class RunCommand : public Task
{
  const mongo::BSONObj& cmd;
  mongo::BSONObj& ret;
  boost::unique_future<bool>& future;
  boost::promise<bool> promise;
public:
  RunCommand(const mongo::BSONObj& cmd, mongo::BSONObj& ret, 
    boost::unique_future<bool>& future) : cmd(cmd), ret(ret), future(future)
  {
    future = promise.get_future();
  }
  void Execute(Worker& worker);
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

  void Execute(Worker& worker);
};

class Delete : public Task
{
  std::string container;
  mongo::Query query;
public:
  Delete(const std::string& container, const mongo::Query& query) :
    container(container), query(query) {};
  void Execute(Worker& worker);
};

class Insert : public Task
{
  std::string container;
  mongo::BSONObj obj;
public:
  Insert(const std::string& container, const mongo::BSONObj& obj) :
    container(container), obj(obj) {};
  void Execute(Worker& worker);
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
  void Execute(Worker& worker);
};


// end
}
#endif
