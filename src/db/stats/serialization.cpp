#include <mongo/client/dbclient.h>
#include "db/stats/serialization.hpp"
#include "stats/date.hpp"
#include "stats/stat.hpp"
#include "cfg/get.hpp"
#include "util/verify.hpp"
#include "stats/types.hpp"
#include "db/error.hpp"

namespace db { namespace stats
{

mongo::BSONObj Serialize(::stats::Timeframe timeframe)
{
  ::stats::Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
  mongo::BSONObjBuilder bob;
  
  switch (timeframe)
  {
    case ::stats::Timeframe::Alltime :
      break;
    case ::stats::Timeframe::Year    :
      bob.append("year", date.Year());
      break;
    case ::stats::Timeframe::Month   :
      bob.append("year", date.Year());
      bob.append("month", date.Month());
      break;
    case ::stats::Timeframe::Week    :
      bob.append("year", date.Year());
      bob.append("week", date.Week());
      break;
    case ::stats::Timeframe::Day     :
      bob.append("year", date.Year());
      bob.append("month", date.Month());
      bob.append("day", date.Day());
      break;
    default                          :
      verify(false);
  }

  return bob.obj();
}

::stats::Stat Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    return ::stats::Stat(obj["_id"].Int(),
                         obj["total files"].Int(),
                         obj["total kbytes"].Long(),
                         obj["total xfertime"].Long());
  } 
  catch (const mongo::DBException& e)
  {
    LogException("Unserialize stats", e, obj);
    throw e;
  }
}

} /* stats namespace */
} /* db namespace */