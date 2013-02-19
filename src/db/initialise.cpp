#include "db/initialise.hpp"
#include "db/connection.hpp"
#include "db/replicator.hpp"
#include "db/usercache.hpp"
#include "db/user.hpp"

namespace db
{

bool CreateUpdateLog()
{
  try
  {
    SafeConnection conn;
    mongo::BSONObj info;
    conn.RunCommand(BSON("create" << "updatelog" << 
                         "capped" << true << 
                         "size" << 102400 << 
                         "max" << 100), info);
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool EnsureIndexes()
{
  try
  {
    SafeConnection conn;
    conn.EnsureIndex("users", BSON("uid" << 1), true);
    conn.EnsureIndex("users", BSON("name" << 1), true);
    conn.EnsureIndex("groups", BSON("gid" << 1), true);
    conn.EnsureIndex("groups", BSON("name" << 1), true);
    conn.EnsureIndex("index", BSON("path" << 1), true);
    conn.EnsureIndex("dupe", BSON("directory" << 1), true);
    conn.EnsureIndex("updatelog", BSON("timestamp" << 1), false);
    conn.EnsureIndex("transfers", BSON("uid" << 1 << 
                                       "direction" << 1 << 
                                       "section" << 1 << 
                                       "day" << 1 << 
                                       "week" << 1 << 
                                       "month" << 1 << 
                                       "year" << 1), true);
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool StartReplication()
{
  try
  {
    /*auto userCache = std::make_shared<UserCache>();
    Replicator::Register(userCache);
    EnableUserCache(userCache);*/
    
    return true;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

bool Initialise()
{
  if (!CreateUpdateLog())
  {
    logs::db << "Error while creating update log" << logs::endl;
    return false;
  }

  if (!EnsureIndexes())
  {
    logs::db << "Error while building database indexes" << logs::endl;
    return false;
  }

  if (!StartReplication())
  {
    logs::db << "Error while initialising database replication" << logs::endl;
    return false;
  }
  
  return true;
}

} /* db namespace */
