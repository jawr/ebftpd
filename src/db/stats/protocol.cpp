#include "db/stats/protocol.hpp"
#include "db/stats/date.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "util/verify.hpp"
#include "cfg/get.hpp"
#include "db/bson/error.hpp"

namespace db { namespace stats
{

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes)
{
  Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
  
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

mongo::BSONObj TimeframeToBSON(::stats::Timeframe timeframe)
{
  Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
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

ProtocolTotal CalculateUser(acl::UserID uid, ::stats::Timeframe timeframe)
{
  mongo::BSONObj cmd = BSON("aggregate" << "protocol" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << TimeframeToBSON(timeframe)) <<
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

  ProtocolTotal total;
  try
  {
    total = ProtocolTotal(result["send total"].Long(), 
                          result["receive total"].Long());
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("protocol total", e, result, true);
  }
  
  return total;
}

ProtocolTotal CalculateTotal(::stats::Timeframe timeframe)
{
  return CalculateUser(-1, timeframe);
}

} /* stats namespace */
} /* db namespace */