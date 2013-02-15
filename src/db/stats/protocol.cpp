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
  ::stats::Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
  
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
