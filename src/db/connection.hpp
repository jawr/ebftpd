//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __DB_CONNECTION_HPP
#define __DB_CONNECTION_HPP

#include <mongo/client/dbclient.h>
#include <boost/thread/once.hpp>
#include <boost/thread/thread.hpp>
#include <boost/optional.hpp>
#include "util/string.hpp"
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
  
  static void InitialiseMongo();
  
public:
  Connection(ConnectionMode mode);
  ~Connection()
  {
    if (scopedConn) scopedConn->done();
  }
  
  mongo::DBClientBase& BaseConn() { return scopedConn->conn(); }
  
  LastError GetLastError()
  {
    return LastError(scopedConn->conn().getLastErrorDetailed());
  }

  int Update(const std::string& collection, const mongo::Query& query, 
          const mongo::BSONObj& obj, bool upsert = false);
  
  template <typename T>
  void SetFields(const std::string& collection, const mongo::Query& query, 
          const T& obj, const std::vector<std::string>& fields)
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
      LogException("Set field", e, typeid(obj), util::Join(fields, " "));
      if (mode == ConnectionMode::Safe) 
        throw DBWriteError();
    }
  }
  
  template <typename T>
  void SetField(const std::string& collection, const mongo::Query& query, const T& obj, 
        const std::string& field)
  {
    SetFields(collection, query, obj, { field });
  }
  
  template <typename BSONObject>
  void Insert(const std::string& collection, const BSONObject& obj)
  {
    if (!scopedConn) return;
    
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      scopedConn->conn().insert(Namespace(collection), obj);
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Insert", err, collection, obj);
          if (mode == ConnectionMode::Safe) throw DBWriteError();
        }
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Insert", e, collection, obj);
      if (mode == ConnectionMode::Safe) throw DBWriteError();
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
      LogException("Insert multi", e, typeid(objects.front()));
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
      LogException("Insert one", e, typeid(obj));
      if (mode == ConnectionMode::Safe) throw e;
    }
  }

  int Remove(const std::string& collection, const mongo::Query& query);
  
  std::vector<mongo::BSONObj> Query(const std::string& collection, 
        const mongo::Query& query, int nToReturn, int nToSkip, 
        const mongo::BSONObj* fieldsToReturn = nullptr);
  
  template <typename T>
  std::vector<T> QueryMulti(const std::string& collection, const mongo::Query& query, 
                            int nToReturn = 0, int nToSkip = 0,
                            const mongo::BSONObj* fieldsToReturn = nullptr)
  {
    std::vector<T> results;
    if (!scopedConn) return results;
    
    auto objects = Query(collection, query, nToReturn, nToSkip, fieldsToReturn);
    try
    {
      std::for_each(objects.begin(), objects.end(),
              [&](const mongo::BSONObj& obj)
              {
                results.emplace_back(Unserialize<T>(obj));
              });
    }
    catch (const mongo::DBException& e)
    {
      LogException("Query multi", e, collection, query, fieldsToReturn);
      if (mode == ConnectionMode::Safe) throw e;
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
    return boost::none;
  }
  
  void EnsureIndex(const std::string& collection, const mongo::BSONObj& keys, bool unique);
  long long Count(const std::string& collection, const mongo::BSONObj& query = mongo::BSONObj());  
          
  bool RunCommand(const mongo::BSONObj& command, mongo::BSONObj& info, int options = 0);
  bool Eval(const std::string& javascript, mongo::BSONObj& info, mongo::BSONElement& ret, 
        mongo::BSONObj* args = nullptr);
  int NextAutoIncrement(const std::string& collection, const std::string& autoIncField);
  int InsertAutoIncrement(const std::string& collection, const mongo::BSONObj& obj, 
        const std::string& autoIncField);
  
  template <typename T>
  int InsertAutoIncrement(const std::string& collection, const T& obj, 
        const std::string& autoIncField)
  {
    if (scopedConn)
    {
      try
      {
        return InsertAutoIncrement(collection, Serialize(obj), autoIncField);
      }
      catch (const mongo::DBException& e)
      {
        LogException("Insert auto increment", e, typeid(obj));
        if (mode == ConnectionMode::Safe) throw e;
      }
    }
    return -1;
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

std::ostream& operator<<(std::ostream& os, const mongo::BSONObj* obj);

} /* db namespace */

#endif
