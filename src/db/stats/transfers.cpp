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

#include "db/stats/transfers.hpp"
#include "db/error.hpp"
#include "db/stats/serialization.hpp"
#include "cfg/get.hpp"
#include "stats/types.hpp"
#include "db/stats/traffic.hpp"
#include "db/connection.hpp"

namespace db { namespace stats
{

long long TransfersUser(acl::UserID uid, ::stats::Timeframe timeframe, 
      const std::string& section, ::stats::Direction direction)
{
  mongo::BSONObjBuilder match;
  match.append("direction", util::EnumToString(direction));
  if (!section.empty())
    match.append("section", section);
  else
  {
    mongo::BSONArrayBuilder sections;
    for (const auto& kv : cfg::Get().Sections())
      sections.append(kv.first);
    match.appendElements(BSON("section" << BSON("$nin" << sections.arr())));
  }
  match.appendElements(Serialize(timeframe));
  
  mongo::BSONObj cmd = BSON("aggregate" << "transfers" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << match.obj()) <<
      BSON("$group" << 
        BSON("_id" << (uid == -1 ? "" : "$uid") <<
          "total" << BSON("$sum" << "$kbytes")
        ))));
  
  
  mongo::BSONObj result;
  NoErrorConnection conn;
  if (conn.RunCommand(cmd, result))
  {
    try
    {
      auto elems = result["result"].Array();
      if (!elems.empty())
      {
        return elems[0]["total"].Long();
      }
    }
    catch (const mongo::DBException& e)
    {
      LogException("Unserialize transfers total", e, result);
    }
  }
  
  return 0;
}

Traffic TransfersUser(acl::UserID uid, ::stats::Timeframe timeframe,
      const std::string& section)
{
  return Traffic(TransfersUser(uid, timeframe, section, ::stats::Direction::Download),
                 TransfersUser(uid, timeframe, section, ::stats::Direction::Upload));
}

Traffic TransfersTotal(::stats::Timeframe timeframe, const std::string& section)
{
  return TransfersUser(-1, timeframe, section);
}

} /* stats namespace */
} /* db namespace */
