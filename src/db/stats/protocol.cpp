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

#include "db/stats/protocol.hpp"
#include "stats/date.hpp"
#include "cfg/get.hpp"
#include "db/stats/traffic.hpp"
#include "stats/types.hpp"
#include "db/stats/serialization.hpp"
#include "db/connection.hpp"

namespace db { namespace stats
{

void ProtocolUpdate(acl::UserID uid, long long sendKBytes, long long receiveKBytes)
{
  ::stats::Date date;
  
  mongo::BSONObjBuilder qbob;
  qbob.append("uid", uid);
  qbob.append("day", date.Day());
  qbob.append("week", date.Week());
  qbob.append("month", date.Month());
  qbob.append("year", date.Year());
  mongo::Query query(qbob.obj());
  
  mongo::BSONObj obj = BSON("$inc" << BSON("send kbytes" << sendKBytes) <<
                            "$inc" << BSON("receive kbytes" << receiveKBytes));
  NoErrorConnection conn;
  conn.Update("protocol", query, obj, true);
}

Traffic ProtocolUser(acl::UserID uid, ::stats::Timeframe timeframe)
{
  mongo::BSONObj cmd = BSON("aggregate" << "protocol" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << Serialize(timeframe)) <<
      BSON("$group" << 
        BSON("_id" << (uid == -1 ? "" : "$uid") <<
          "send total" << BSON("$sum" << "$send kbytes") <<
          "receive total" << BSON("$sum" << "$receive kbytes")
        ))));
  
  mongo::BSONObj result;
  NoErrorConnection conn;
  if (conn.RunCommand(cmd, result))
  {
    auto elems = result["result"].Array();  
    if (!elems.empty())
    {
      try
      {
        return Traffic(elems[0]["send total"].Long(), 
                       elems[0]["receive total"].Long());
      }
      catch (const mongo::DBException& e)
      {
        LogException("Unserialize protocol total", e, result);
      }
    }
  }
  
  return Traffic();
}

Traffic ProtocolTotal(::stats::Timeframe timeframe)
{
  return ProtocolUser(-1, timeframe);
}

} /* stats namespace */
} /* db namespace */
