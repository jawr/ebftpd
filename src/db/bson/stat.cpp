#include <vector>
#include "db/bson/stat.hpp"
#include "db/bson/bson.hpp"
#include "stats/stat.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

mongo::BSONObj Stat::Serialize(const ::stats::Stat& stat)
{
  mongo::BSONObjBuilder bob;
  bob.append("uid", stat.uid);
  bob.append("day", stat.day);
  bob.append("month", stat.month);
  bob.append("week", stat.week);
  bob.append("year", stat.year);
  bob.append("files", stat.files);
  bob.append("kbytes ", stat.kbytes);
  bob.append("xfertime", stat.xfertime);
  if (stat.direction == ::stats::Direction::Upload)
    bob.append("direction", "up");
  else
    bob.append("direction", "dn");
  return bob.obj();
}

::stats::Stat Stat::UnserializeRaw(const mongo::BSONObj& bo)
{
  ::stats::Stat stat;
  try
  {
    stat.uid = bo["_id"].Int();
    stat.files = bo["files"].Int();
    stat.kbytes = bo["kbytes"].Long();
    stat.xfertime = bo["xfertime"].Long();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("stat raw", e, bo);
  }

  return stat;
}

::stats::Stat Stat::Unserialize(const mongo::BSONObj& bo)
{
  ::stats::Stat stat;

  try
  {
    stat.uid = bo["uid"].Int(); 

    stat.files = bo["files"].Int();
    stat.kbytes = bo["kbytes"].Long();
    stat.xfertime = bo["xfertime"].Long();

    if (bo["direction"].String() == "up")
      stat.direction = ::stats::Direction::Upload;
    else
      stat.direction = ::stats::Direction::Download;

    stat.day = bo["day"].Int();
    stat.week = bo["week"].Int();
    stat.month = bo["month"].Int();
    stat.year = bo["year"].Int();
  } 
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("stat", e, bo);
  }

  return stat;
}
  
} /* bson namespace */
} /* db namespace */

