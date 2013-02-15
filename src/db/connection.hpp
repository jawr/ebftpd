#ifndef __DB_CONNECTION_HPP
#define __DB_CONNECTION_HPP

#include <mongo/client/dbclient.h>
#include <boost/thread/once.hpp>
#include <boost/thread/thread.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/join.hpp>
#include <memory>
#include <utility>
#include <sstream>
#include <vector>
#include <typeinfo>
#include "logs/logs.hpp"
#include "db/serialization.hpp"
#include "db/error.hpp"

namespace db
{

enum class ConnectionMode
{
  Safe,
  NoError,
  Fast
};

class Connection
{
  struct AuthenticateHook : mongo::DBConnectionHook
  {
    void onCreate(mongo::DBClientBase* conn);
    void onHandedOut(mongo::DBClientBase*) { }
    void onDestroy(mongo::DBClientBase*) { }
  };

  std::unique_ptr<mongo::ScopedDbConnection> scopedConn;
  ConnectionMode mode;
  std::string database;
  
  void Create();
  
  std::string Namespace(const std::string& collection)
  {
    std::string ns(database);
    ns += '.';
    ns += collection;
    return ns;
  }
  
  static boost::once_flag once;
  
  static void CreateAuthenticateHook();
  
public:
  Connection(ConnectionMode mode);

  int Update(const std::string& collection, const mongo::Query& query, const mongo::BSONObj& obj, bool upsert = false)
  {
    if (scopedConn)
    {
      boost::this_thread::disable_interruption noInterrupt;
      
      try
      {
        scopedConn->conn().update(Namespace(collection), query, obj, upsert);
        if (mode != ConnectionMode::Fast)
        {
          return scopedConn->conn().getLastErrorDetailed()["n"].Int();
        }
      }
      catch (const mongo::DBException& e)
      {
        LogException("Update", e, collection, query, obj, upsert);
        if (mode == ConnectionMode::Safe) throw e;
      }
    }
    return 0;
  }

  template <typename T>
  void SetField(const std::string& collection, const mongo::Query& query, const T& obj, const std::vector<std::string>& fields)
  {
    try
    {
      auto bsonObj = Serialize(obj);
      mongo::BSONObjBuilder bob;
      for (const std::string& field : fields)
      {
        bob.append("$set", BSON(field << bsonObj[field]));
      }
      Update(collection, query, bob.obj());
    }
    catch (const mongo::DBException& e)
    {
      LogException("SetField", e, typeid(obj).name(), boost::join(fields, " "));
      if (mode == ConnectionMode::Safe) throw e;
    }
  }
  
  template <typename T>
  void SetField(const std::string& collection, const mongo::Query& query, const T& obj, const std::string& field)
  {
    SetField(collection, query, obj, { field });
  }
  
  template <typename BSONObject>
  void Insert(const std::string& collection, const BSONObject& obj)
  {
    if (!scopedConn) return;
    
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      scopedConn->conn().insert(Namespace(collection), obj);
    }
    catch (const mongo::DBException& e)
    {
      LogException("Insert", e, collection, obj);
      if (mode == ConnectionMode::Safe) throw e;
    }
  }
  
  template <typename T>
  void InsertMulti(const std::string& collection, const std::vector<T>& objects)
  {
    if (!scopedConn || objects.empty()) return;
    
    try
    {
      std::vector<mongo::BSONObj> bsonObjects;
      std::for_each(objects.begin(), objects.end(),
              [&](const T& obj)
              {
                bsonObjects.emplace_back(Serialize<T>(obj));
              });
      Insert(collection, bsonObjects);
    }
    catch (const mongo::DBException& e)
    {
      LogException("InsertOne", e, typeid(objects.front()).name());
      if (mode == ConnectionMode::Safe) throw e;
    }    
  }
    
  template <typename T>
  void InsertOne(const std::string& collection, const T& obj)
  {
    if (!scopedConn) return;
    try
    {
      Insert(collection, Serialize(obj));
    }
    catch (const mongo::DBException& e)
    {
      LogException("InsertOne", e, typeid(obj).name());
      if (mode == ConnectionMode::Safe) throw e;
    }
  }

  int Remove(const std::string& collection, const mongo::Query& query)
  {
    if (!scopedConn) return 0;
    
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      scopedConn->conn().remove(Namespace(collection), query);
      if (mode != ConnectionMode::Fast)
      {
        return scopedConn->conn().getLastErrorDetailed()["n"].Int();
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Remove", e, collection, query);
      if (mode == ConnectionMode::Safe) throw e;
    }
    return 0;
  }
  
  std::unique_ptr<mongo::DBClientCursor> 
  Query(const std::string& collection, const mongo::Query& query, int nToReturn = 0, int nToSkip = 0,
        const mongo::BSONObj* fieldsToReturn = nullptr)
  {
    if (!scopedConn) return nullptr;
    
    try
    {
      return std::unique_ptr<mongo::DBClientCursor>(scopedConn->conn().query(
                Namespace(collection), query, nToReturn, nToSkip, fieldsToReturn));
    }
    catch (const mongo::DBException& e)
    {
      LogException("Query", e, collection, query, nToReturn, nToSkip, *fieldsToReturn);
      if (mode == ConnectionMode::Safe) throw e;
      else return nullptr;
    }
  }
  
  template <typename T>
  std::vector<T> QueryMulti(const std::string& collection, const mongo::Query& query, 
                            int nToReturn = 0, int nToSkip = 0,
                            const mongo::BSONObj* fieldsToReturn = nullptr)
  {
    std::vector<T> results;
    if (!scopedConn) return results;
    
    auto cursor = Query(collection, query, nToReturn, nToSkip, fieldsToReturn);
    if (cursor)
    {
      try
      {
        while (cursor->more())
        {
          results.emplace_back(Unserialize<T>(cursor->nextSafe()));
        }
      }
      catch (const mongo::DBException& e)
      {
        LogException("QueryMulti", e, collection, query, fieldsToReturn);
        if (mode == ConnectionMode::Safe) throw e;
      }
    }
    
    return results;
  }

  template <typename T>
  boost::optional<T> QueryOne(const std::string& collection, const mongo::Query& query, 
                              const mongo::BSONObj* fieldsToReturn = nullptr)
  {
    if (scopedConn)
    {
      auto results = QueryMulti<T>(collection, query, 1, 0, fieldsToReturn);
      if (!results.empty()) return boost::optional<T>(results.front());
    }
    return boost::optional<T>();
  }
  
  void EnsureIndex(const std::string& collection, const mongo::BSONObj& keys, bool unique)
  {
    if (!scopedConn) return;
    
    try
    {
      scopedConn->conn().ensureIndex(collection, keys, unique);
    }
    catch (const mongo::DBException& e)
    {
      LogException("EnsureIndex", e, collection, keys, unique);
      if (mode == ConnectionMode::Safe) throw e;
    }
  }
  
  unsigned long long Count(const std::string& collection, const mongo::BSONObj& query = mongo::BSONObj())
  {
    if (scopedConn) 
    {
      try
      {
        return scopedConn->conn().count(Namespace(collection), query);
      }
      catch (const mongo::DBException& e)
      {
        LogException("Count", e, collection, query);
        if (mode == ConnectionMode::Safe) throw e;
      }
    }
    
    return 0;
  }
  
  bool RunCommand(const mongo::BSONObj& command, mongo::BSONObj& info, int options = 0)
  {
    if (scopedConn)
    {
      boost::this_thread::disable_interruption noInterrupt;
      
      try
      {
        return scopedConn->conn().runCommand(database, command, info, options);
      }
      catch (const mongo::DBException& e)
      {
        LogException("RunCommand", e, database, command, "output reference", options);
        if (mode == ConnectionMode::Safe) throw e;
      }      
    }
    
    return false;
    
  }
};

class SafeConnection : public Connection
{
public:
  SafeConnection() : Connection(ConnectionMode::Safe) { }
};

class NoErrorConnection : public Connection
{
public:
  NoErrorConnection() : Connection(ConnectionMode::NoError) { }
};

class FastConnection : public Connection
{
public:
  FastConnection() : Connection(ConnectionMode::Fast) { }
};

} /* db namespace */

#endif
