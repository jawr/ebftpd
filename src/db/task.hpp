#ifndef __DB_TASK_HPP
#define __DB_TASK_HPP

#include <mongo/client/dbclient.h>
#include <boost/thread/future.hpp>
#include "db/types.hpp"

namespace db
{

class Task
{
protected:
  const std::string database;
  
public:
  Task();
  virtual ~Task() {}
  virtual void Execute(mongo::DBClientConnection& conn) = 0;
};

class EnsureIndex : public Task
{
  std::string collection;
  mongo::BSONObj obj;

public:
  EnsureIndex(const std::string& collection, const mongo::BSONObj& obj) :
      collection(collection), obj(obj) {}
  void Execute(mongo::DBClientConnection& conn);
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
  void Execute(mongo::DBClientConnection& conn);
};

class Select : public Task
{
  std::string collection;
  const mongo::Query& query;
  QueryResults& results;
  boost::unique_future<bool>& future;
  boost::promise<bool> promise;
  int limit;
public:
  Select(const std::string& collection, const mongo::Query& query,
    QueryResults& results, boost::unique_future<bool>& future, int limit = 0) : 
      collection(collection), query(query), results(results), future(future),
      promise(), limit(limit) { future = promise.get_future(); }

  void Execute(mongo::DBClientConnection& conn);
};

class Delete : public Task
{
  std::string collection;
  mongo::Query query;
public:
  Delete(const std::string& collection, const mongo::Query& query) :
    collection(collection), query(query) {}
  void Execute(mongo::DBClientConnection& conn);
};

class Insert : public Task
{
  std::string collection;
  mongo::BSONObj obj;
public:
  Insert(const std::string& collection, const mongo::BSONObj& obj) :
    collection(collection), obj(obj) {}
  void Execute(mongo::DBClientConnection& conn);
};

class Update : public Task
{
protected:
  std::string collection;
  mongo::BSONObj obj;
  mongo::Query query;
  bool upsert;
public:
  Update(const std::string& collection, const mongo::Query& query, 
    const mongo::BSONObj& obj, bool upsert=false) :
    collection(collection), obj(obj), query(query), upsert(upsert) {}
  void Execute(mongo::DBClientConnection& conn);
};


// end
}
#endif
