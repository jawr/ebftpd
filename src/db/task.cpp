#include <boost/thread/future.hpp>
#include "db/task.hpp"
#include "db/worker.hpp"
#include "db/error.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

namespace db
{

Task::Task() : database(cfg::Get().Database().Name()) { }

bool RunCommand::Execute(mongo::DBClientConnection& conn,
          const mongo::BSONObj& cmd, mongo::BSONObj& ret)
{
  const std::string& database = cfg::Get().Database().Name();
  try
  {
    conn.runCommand(database, cmd, ret);
    return true;
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Command failed: " << database << " : " << cmd.toString() 
             << " : " << e.what() << logs::endl;
    return false;
  }
}

void RunCommand::Execute(mongo::DBClientConnection& conn)
{
  promise.set_value(Execute(conn, cmd, ret));
}

int Update::Execute(mongo::DBClientConnection& conn, const std::string& collection, 
                    const mongo::Query& query, const mongo::BSONObj& obj, bool upsert)
{
  const std::string& database = cfg::Get().Database().Name();
  try
  {
    conn.update(database + "." + collection, query, obj, upsert, false);
    LastErrorToException(conn);
    return conn.getLastErrorDetailed()["n"].Int();
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Update failed: " << database << "." 
             << collection << " : " << query.toString() 
             << " : " << obj.toString() << " upsert=" 
             << upsert << " : " << e.what() << logs::endl;
    return -1;
  }  
}

void Update::Execute(mongo::DBClientConnection& conn)
{
  promise.set_value(Execute(conn, collection, query, obj, upsert));
}

void Delete::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    conn.remove(database + "." + collection, query);
    LastErrorToException(conn);
    promise.set_value(conn.getLastErrorDetailed()["n"].Int());
  }
  catch (const mongo::DBException& e)
  {
    promise.set_value(-1);
    logs::db << "Delete failed: " << database << "." << collection 
             << " : " << query.toString() 
             << " : " << e.what() << logs::endl;
  }
}

void Select::Execute(mongo::DBClientConnection& conn)
{
  results.clear();
  if (limit) results.reserve(limit);
  
  try
  {
    std::unique_ptr<mongo::DBClientCursor> cursor =
      conn.query(database + "." + collection, query, limit, skip);

    while (cursor->more())
    {
      results.push_back(cursor->nextSafe().copy());
    }
    
    LastErrorToException(conn);
    promise.set_value(true);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Select query failed: " << database << "." << collection 
             << " : " << query.toString() << " : limit= " << limit 
             << " : " << "skip=" << skip
             << " : " << e.what() << logs::endl;
    promise.set_value(false);
  }
  
  //results.shrink_to_fit(); // segment fault on gcc 4.7.1
}

void Insert::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    conn.insert(database + "." + collection, obj);
    LastErrorToException(conn);
  }
  catch (const mongo::DBException& e)
  {
    if (!failOkay)
      logs::db << "Insert failed: " << database << "." << collection 
               << " : " << obj.toString() << " : " << e.what() << logs::endl;
  }
}

void EnsureIndex::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    conn.ensureIndex(database + "." + collection, obj, true);
    LastErrorToException(conn);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Ensure index failed: " << database << "." << collection
             << " : " << obj.toString() << " : " << e.what() << logs::endl;
  }
}

std::string Eval::SimplifyJavascript()
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

void Eval::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    mongo::BSONObj info;
    if (!conn.eval(database, javascript, info, ret, &args))
      LastErrorToException(conn);
    promise.set_value(true);
  }
  catch (const mongo::DBException& e)
  {
    promise.set_value(false);
    logs::db << "Eval failed: " << database << ":" 
             << SimplifyJavascript() << " : " << e.what() << logs::endl;
  }
}

// end
}
