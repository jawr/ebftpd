#include <cassert>
#include <boost/thread/once.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "util/string.hpp"
#include "db/connection.hpp"
#include "cfg/get.hpp"
#include "db/error.hpp"

namespace db
{

namespace
{

std::string SimplifyJavascript(const std::string& javascript)
{
  std::string simple = boost::replace_all_copy(javascript, "\n", " ");
  util::string::CompressWhitespace(simple);
  if (boost::starts_with(simple, "function "))
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
      throw db::DBError("Authentication error");
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
  boost::call_once(&CreateAuthenticateHook, once);  
  
  try
  {
    try
    {
      scopedConn.reset(mongo::ScopedDbConnection::getScopedDbConnection(cfg::Get().Database().Host()));
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
  
  if (mode == ConnectionMode::Fast)
    scopedConn->conn().setWriteConcern(mongo::W_NONE);
}

void Connection::CreateAuthenticateHook()
{
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
      const mongo::BSONObj* fieldsToReturn = nullptr)
{
  std::vector<mongo::BSONObj> results;
  if (scopedConn)
  {
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
        results.emplace_back(cursor->next());
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Query", e, collection, query, nToReturn, nToSkip, *fieldsToReturn);
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
    scopedConn->conn().ensureIndex(collection, keys, unique);
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


unsigned long long Connection::Count(const std::string& collection, 
        const mongo::BSONObj& query)
{
  unsigned long long count = 0;
  if (scopedConn) 
  {
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
        }
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Count", e, collection, query);
      if (mode == ConnectionMode::Safe) throw DBReadError();
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
      if (!ret && mode != ConnectionMode::Fast)
      {
        auto err = GetLastError();
        verify(!err.Okay()); // should be same as ret
        LogLastError("Run command", err, command, "output ref", options);
        if (mode == ConnectionMode::Safe) throw DBError();
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

bool Connection::Eval(const std::string& javascript, mongo::BSONElement& retval, 
      mongo::BSONObj* args)
{
  bool ret = false;
  if (scopedConn)
  {
    try
    {
      mongo::BSONObj info;
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
  
  static const char* javascript =
    "function autoIncInsert(colName, field, obj) {\n"
    "  var col = db[colName];\n"
    "  var fieldsToReturn = {};\n"
    "  fieldsToReturn[field] = 1;\n"
    "  var sort = {};\n"
    "  sort[field] = -1;\n"
    "  while (1) {\n"
    "    var cursor = col.find({}, fieldsToReturn).sort(sort).limit(1);\n"
    "    obj[field] = cursor.hasNext() ? cursor.next()[field] + 1 : 0;\n"
    "    col.insert(obj);\n"
    "    var err = db.getLastErrorObj();\n"
    "    if (err &&  err.code == 11000) {\n"
    "      var query = {};\n"
    "      query[field] = obj[field];\n"
    "      if (col.findOne(query))\n"
    "        continue;\n"
    "      else\n"
    "        return -1;\n"
    "    }\n"
    "    break;\n"
    "  }\n"
    "  return obj[field];\n"
    "}\n";
  
  mongo::BSONArrayBuilder bab;
  bab.append(collection);
  bab.append(autoIncField);
  bab.append(obj);
  auto args = bab.arr();
  
  mongo::BSONElement ret;
  if (!Eval(javascript, ret, &args)) return -1;
  return ret.Int();
}

std::ostream& operator<<(std::ostream& os, const mongo::BSONObj* obj)
{
  if (obj) os << *obj;
  else os << "NULL";
  return os;
}
  
} /* db namespace */
