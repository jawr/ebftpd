#include <vector>
#include "db/bson/stat.hpp"
#include "db/bson/bson.hpp"
#include "stats/stat.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

::stats::Stat Stat::Unserialize(const mongo::BSONObj& bo)
{
  ::stats::Stat stat;
  try
  {
    stat.id = bo["_id"].Int(); 
    stat.files = bo["total files"].Int();
    stat.bytes = bo["total bytes"].Long();
    stat.xfertime = bo["total xfertime"].Long();
    stat.speed = -1;
  } 
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("stat", e, bo);
  }
  return stat;
}

} /* bson namespace */
} /* db namespace */

