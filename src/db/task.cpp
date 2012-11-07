#include <boost/thread/future.hpp>
#include "db/task.hpp"
#include "db/worker.hpp"
#include "db/exception.hpp"
#include "logs/logs.hpp"
#include "cfg/get.hpp"

namespace db
{

Task::Task() : database(cfg::Get().Database().Name()) { }

void RunCommand::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    if (conn.runCommand(database, cmd, ret))
      ret = ret.getObjectField("result").getObjectField("0");
    promise.set_value(true);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Query failed: " << database << " : " << cmd.toString() 
             << " : " << e.what() << logs::endl;
    promise.set_value(false);
  }
}

void Update::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    conn.update(database + "." + collection, query, obj, upsert, false);
    LastErrorToException(conn);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Update failed: " << database << "." 
             << collection << " : " << query.toString() 
             << " : " << obj.toString() << " upsert=" 
             << upsert << " : " << e.what() << logs::endl;
  }
}

void Delete::Execute(mongo::DBClientConnection& conn)
{
  try
  {
    conn.remove(database + "." + collection, query);
    LastErrorToException(conn);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Delete failed: " << database << "." << collection 
             << " : " << query.toString() 
             << " : " << e.what() << logs::endl;
  }
}

void Select::Execute(mongo::DBClientConnection& conn)
{

  results.clear();
  results.reserve(limit);
  
  try
  {
    std::unique_ptr<mongo::DBClientCursor> cursor =
      conn.query(database + "." + collection, query);
    int i = 0;
    while (cursor->more())
    {
      results.push_back(cursor->nextSafe().copy());
      if (limit != 0 && ++i == limit) break;
    }
    LastErrorToException(conn);
    promise.set_value(true);
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Select query failed: " << database << "." << collection 
             << " : " << query.toString() << " : limit= " << limit 
             << " : " << e.what() << logs::endl;
    promise.set_value(false);
  }
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


// end
}
