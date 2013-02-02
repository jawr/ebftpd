#include <mongo/client/dbclient.h>
#include "db/bson/timeframe.hpp"
#include "stats/date.hpp"
#include "cfg/get.hpp"
#include "util/verify.hpp"
#include "stats/types.hpp"

namespace db { namespace bson
{

mongo::BSONObj TimeframeSerialize(::stats::Timeframe timeframe)
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

} /* bson namespace */
} /* db namespace */