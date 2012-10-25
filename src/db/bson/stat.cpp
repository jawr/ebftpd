#include <vector>
#include "db/bson/stat.hpp"
#include "db/bson/bson.hpp"
#include "stats/stat.hpp"
#include "logs/logs.hpp"

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

::stats::Stat Stat::Unserialize(const mongo::BSONObj& bo)
{
  ::stats::Stat stat;
 
  stat.uid = bo["uid"].Int(); 
  stat.day = bo["day"].Int();
  stat.week = bo["week"].Int();
  stat.month = bo["month"].Int();
  stat.year = bo["year"].Int();

  stat.files = bo["files"].Int();
  stat.kbytes = bo["kbytes"].Long();
  stat.xfertime = bo["kbytes"].Double();

  if (bo["direction"].String() == "up")
    stat.direction = ::stats::Direction::Upload;
  else
    stat.direction = ::stats::Direction::Download;

  return stat;
}
  
} /* bson namespace */
} /* db namespace */

