#include "db/stats/transfers.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/bson/error.hpp"
#include "db/bson/timeframe.hpp"
#include "cfg/get.hpp"

namespace db { namespace stats
{

long long TransfersUser(acl::UserID uid, ::stats::Timeframe timeframe, 
      const std::string& section, const std::string& direction)
{
  mongo::BSONObjBuilder match;
  match.append("direction", direction);
  if (!section.empty())
    match.append("section", section);
  else
  {
    mongo::BSONArrayBuilder sections;
    for (const auto& kv : cfg::Get().Sections())
      sections.append(kv.first);
    match.appendElements(BSON("section" << BSON("$nin" << sections.arr())));
  }
  match.appendElements(::db::bson::TimeframeSerialize(timeframe));
  
  mongo::BSONObj cmd = BSON("aggregate" << "transfers" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << match.obj()) <<
      BSON("$group" << 
        BSON("_id" << (uid == -1 ? "" : "$uid") <<
          "total" << BSON("$sum" << "$bytes")
        ))));
  
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();

  long long total = 0;
  try
  {
    total = result["total"].Long();
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("transfers total", e, result, true);
  }
  
  return total;
}

Traffic TransfersUser(acl::UserID uid, ::stats::Timeframe timeframe,
      const std::string& section)
{
  return Traffic(TransfersUser(uid, timeframe, section, "dn"),
                 TransfersUser(uid, timeframe, section, "up"));
}

Traffic TransfersTotal(::stats::Timeframe timeframe, const std::string& section)
{
  return TransfersUser(-1, timeframe, section);
}

} /* stats namespace */
} /* db namespace */