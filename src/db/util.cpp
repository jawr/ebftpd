#include "db/util.hpp"
#include "db/connection.hpp"

namespace db
{

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
    conn.EnsureIndex("transfers", BSON("uid" << 1 << 
                                       "direction" << 1 << 
                                       "section" << 1 << 
                                       "day" << 1 << 
                                       "week" << 1 << 
                                       "month" << 1 << 
                                       "year" << 1), true);
    return true;
  }
  catch (const mongo::DBException& e)
  { }
  catch (const DBError&)
  { }
  
  return false;
}

} /* db namespace */
