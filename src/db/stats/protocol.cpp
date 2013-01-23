#include "db/stats/protocol.hpp"
#include "stats/date.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "cfg/get.hpp"
#include "db/bson/error.hpp"
#include "db/bson/timeframe.hpp"

namespace db { namespace stats
{

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes)
{
  ::stats::Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
  
  mongo::BSONObjBuilder qbob;
  qbob.append("uid", uid);
  qbob.append("day", date.Day());
  qbob.append("week", date.Week());
  qbob.append("month", date.Month());
  qbob.append("year", date.Year());
  mongo::Query query(qbob.obj());
  
  mongo::BSONObj bo = BSON("$inc" << BSON("send bytes" << sendBytes) <<
                           "$inc" << BSON("receive bytes" << receiveBytes));
  TaskPtr task(new db::Update("protocol", query, bo, true));
  Pool::Queue(task);
}

Traffic ProtocolUser(acl::UserID uid, ::stats::Timeframe timeframe)
{
  mongo::BSONObj cmd = BSON("aggregate" << "protocol" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << ::db::bson::TimeframeSerialize(timeframe)) <<
      BSON("$group" << 
        BSON("_id" << (uid == -1 ? "" : "$uid") <<
          "send total" << BSON("$sum" << "$send bytes") <<
          "receive total" << BSON("$sum" << "$receive bytes")
        ))));
  
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();

  Traffic total;
  if (result.nFields() > 0)
  {
    try
    {
      total = Traffic(result["0"]["send total"].Long(), 
                      result["0"]["receive total"].Long());
    }
    catch (const mongo::DBException& e)
    {
      db::bson::UnserializeFailure("protocol total", e, result, true);
    }
  }
  
  return total;
}

Traffic ProtocolTotal(::stats::Timeframe timeframe)
{
  return ProtocolUser(-1, timeframe);
}

} /* stats namespace */
} /* db namespace */