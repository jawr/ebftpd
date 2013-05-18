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

#include <cassert>
#include <boost/algorithm/string/replace.hpp>
#include "db/connection.hpp"
#include "cfg/get.hpp"

namespace db
{

namespace
{

std::string SimplifyJavascript(const std::string& javascript)
{
  std::string simple = boost::replace_all_copy(javascript, "\n", " ");
  util::CompressWhitespace(simple);
  if (util::StartsWith(simple, "function "))
  {
    auto pos = simple.find_first_of("\t {", 9);
    if (pos != std::string::npos)
    {
      return simple.substr(9, pos - 9);
    }
  }
  return simple;
}

}

boost::once_flag Connection::once = BOOST_ONCE_INIT;

void Connection::AuthenticateHook::onCreate(mongo::DBClientBase* conn)
{
  const auto& dbConfig = cfg::Get().Database();
  if (dbConfig.NeedAuth())
  {
    std::string errmsg;
    if (!conn->auth(dbConfig.Name(), dbConfig.Login(), dbConfig.Password(), errmsg))
    {
      throw db::DBError(errmsg);
    }
  }
}

Connection::Connection(ConnectionMode mode) :
  mode(mode),
  database(cfg::Get().Database().Name())
{
  Create();
}

void Connection::Create()
{
  boost::call_once(&InitialiseMongo, once);  
  boost::this_thread::disable_interruption noInterrupt;
  
  try
  {
    try
    {
      std::string errmsg;
      auto connStr = mongo::ConnectionString::parse(cfg::Get().Database().URL(), errmsg);
      scopedConn.reset(mongo::ScopedDbConnection::getScopedDbConnection(connStr));
      (void) errmsg;
    }
    catch (const mongo::DBException& e)
    {
      LogException("Connect", e);
      if (mode == ConnectionMode::Safe)
        throw DBError("Unable to connect to database");
    }
  }
  catch (const db::DBError& e)
  {
    LogException("Connect", e);
    if (mode == ConnectionMode::Safe)
      throw DBError("Unable to authenticate with database");
  }
  
  if (scopedConn && mode == ConnectionMode::Fast)
    scopedConn->conn().setWriteConcern(mongo::W_NONE);
}

void Connection::InitialiseMongo()
{
  FILE* nullLog = fopen("/dev/null", "w");
  if (nullLog != nullptr) mongo::Logstream::setLogFile(nullLog);
  mongo::pool.addHook(new AuthenticateHook());
}

int Connection::Update(const std::string& collection, const mongo::Query& query, 
      const mongo::BSONObj& obj, bool upsert)
{
  if (scopedConn)
  {
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      scopedConn->conn().update(Namespace(collection), query, obj, upsert);
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Update", err, collection, query, obj, upsert);
          if (mode == ConnectionMode::Safe)
            throw DBWriteError();
        }
        return err.NumChanged();
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Update", e, collection, query, obj, upsert);
      if (mode == ConnectionMode::Safe)
        throw DBWriteError();
    }
  }
  return 0;
}


int Connection::Remove(const std::string& collection, const mongo::Query& query)
{
  if (scopedConn)
  {
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      scopedConn->conn().remove(Namespace(collection), query);
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Remove", err, collection, query);
          if (mode == ConnectionMode::Safe)
            throw DBWriteError();
        }
        return err.NumChanged();
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Remove", e, collection, query);
      if (mode == ConnectionMode::Safe) throw DBWriteError();
    }
  }
  return 0;
}

std::vector<mongo::BSONObj> Connection::Query(
      const std::string& collection, const mongo::Query& query, 
      int nToReturn = 0, int nToSkip = 0,
      const mongo::BSONObj* fieldsToReturn)
{
  std::vector<mongo::BSONObj> results;
  if (scopedConn)
  {
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      auto cursor = scopedConn->conn().query(Namespace(collection), 
            query, nToReturn, nToSkip, fieldsToReturn);
      if (!cursor.get()) throw DBReadError();
      
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Query", err, collection, query, nToReturn, nToSkip, fieldsToReturn);
          if (mode == ConnectionMode::Safe) throw DBReadError();
        }
      }
      
      while (cursor->more())
      {
        results.emplace_back(cursor->next().copy());
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Query", e, collection, query, nToReturn, nToSkip, fieldsToReturn);
      if (mode == ConnectionMode::Safe) throw DBReadError();
    }
  }
  return results;
}


void Connection::EnsureIndex(const std::string& collection, 
      const mongo::BSONObj& keys, bool unique)
{
  if (!scopedConn) return;
  
  try
  {
    boost::this_thread::disable_interruption noInterrupt;

    scopedConn->conn().ensureIndex(Namespace(collection), keys, unique);
    if (mode != ConnectionMode::Fast)
    {
      auto err = GetLastError();
      if (!err.Okay())
      {
        LogLastError("Ensure index", err, collection, keys, unique);
        if (mode == ConnectionMode::Safe) throw DBWriteError();
      }
    }
  }
  catch (const mongo::DBException& e)
  {
    LogException("Ensure index", e, collection, keys, unique);
    if (mode == ConnectionMode::Safe) throw DBWriteError();
  }
}


long long Connection::Count(const std::string& collection, const mongo::BSONObj& query)
{
  long long count = -1;
  if (scopedConn) 
  {
    boost::this_thread::disable_interruption noInterrupt;

    try
    {
      count = scopedConn->conn().count(Namespace(collection), query);
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Count", err, collection, query);
          if (mode == ConnectionMode::Safe) throw DBReadError();
          count = -1;
        }
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Count", e, collection, query);
      if (mode == ConnectionMode::Safe) throw DBReadError();
      count = -1;
    }
  }
  
  return count;
}

bool Connection::RunCommand(const mongo::BSONObj& command, mongo::BSONObj& info, int options)
{
  bool ret = false;
  if (scopedConn)
  {
    boost::this_thread::disable_interruption noInterrupt;
    
    try
    {
      ret = scopedConn->conn().runCommand(database, command, info, options);
      if (mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        if (!err.Okay())
        {
          LogLastError("Run command", err, command, "output ref", options);
          if (mode == ConnectionMode::Safe) throw DBError();
        }
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Run command", e, command, "output ref", options);
      if (mode == ConnectionMode::Safe) throw DBError();
    }      
  }
  
  return ret;    
}

bool Connection::Eval(const std::string& javascript, mongo::BSONObj& info, mongo::BSONElement& retval, 
      mongo::BSONObj* args)
{
  bool ret = false;
  if (scopedConn)
  {
    boost::this_thread::disable_interruption noInterrupt;

    try
    {
      ret = scopedConn->conn().eval(database, javascript, info, retval, args);
      if (!ret && mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        verify(!err.Okay());
        LogLastError("Eval", err, javascript, "info out ref", "retval out ref", args);
        if (mode == ConnectionMode::Safe) throw DBError();
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Eval", e, SimplifyJavascript(javascript), args);
      if (mode == ConnectionMode::Safe) throw DBError();
    }
  }
  return ret;
}

int Connection::InsertAutoIncrement(const std::string& collection, 
      const mongo::BSONObj& obj, const std::string& autoIncField)
{
  if (!scopedConn) return -1;

  std::string ns = Namespace(collection);
  while (true)
  {
    int id = NextAutoIncrement(collection, autoIncField);
    if (id < 0) return -1;

    mongo::BSONObjBuilder bab;
    mongo::BSONObjIterator it(obj);
    while (it.more())
    {
      auto e = it.next();
      if (autoIncField != e.fieldName())
        bab.append(e);
    }
    bab.append(autoIncField, id);

    try
    {
      boost::this_thread::disable_interruption noInterrupt;
      
      scopedConn->conn().insert(ns, bab.obj());
      auto err = GetLastError();
      if (!err.Okay())
      {
        if (err["code"].Number() == 11000)
        {
          auto fields = BSON(autoIncField << 1);
          auto cursor = scopedConn->conn().query(ns, QUERY(autoIncField << id), 1, 0, &fields);
          if (cursor.get() && cursor->more())
            continue;
          else
            return -1;
        }
          
        LogLastError("Insert", err, collection, obj);
        if (mode == ConnectionMode::Safe) throw DBWriteError();
        return -1;
      }
      
      return id;
    }
    catch (const mongo::DBException& e)
    {
      LogException("Insert auto increment", e, collection, obj);
      if (mode == ConnectionMode::Safe) throw DBWriteError();
    }
  }
}

int Connection::NextAutoIncrement(const std::string& collection, const std::string& autoIncField)
{
  if (!scopedConn) return -1;
  
  static const char* javascript =
    "function autoIncInsert(colName, field) {\n"
    "  var col = db[colName];\n"
    "  var fieldsToReturn = {};\n"
    "  fieldsToReturn[field] = 1;\n"
    "  var sort = {};\n"
    "  sort[field] = -1;\n"
    "  var cursor = col.find({}, fieldsToReturn).sort(sort).limit(1);\n"
    "  return cursor.hasNext() ? cursor.next()[field] + 1 : 0;\n"
    "}\n";

  mongo::BSONArrayBuilder bab;
  bab.append(collection);
  bab.append(autoIncField);
  auto args = bab.arr();

  mongo::BSONObj info;
  mongo::BSONElement ret;
  
  if (!Eval(javascript, info, ret, &args)) return -1;
  
  return static_cast<int>(ret.Number());
}

std::ostream& operator<<(std::ostream& os, const mongo::BSONObj* obj)
{
  if (obj) os << *obj;
  else os << "NULL";
  return os;
}
  
} /* db namespace */
