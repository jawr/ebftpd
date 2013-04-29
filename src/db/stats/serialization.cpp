//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
  ::stats::Date date;
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