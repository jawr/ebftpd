#include "db/stats/transfers.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/bson/error.hpp"
#include "db/bson/timeframe.hpp"
#include "cfg/get.hpp"

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
  match.appendElements(::db::bson::TimeframeSerialize(timeframe));
  
  mongo::BSONObj cmd = BSON("aggregate" << "transfers" << "pipeline" <<
    BSON_ARRAY(
      BSON("$match" << match.obj()) <<
      BSON("$group" << 
        BSON("_id" << (uid == -1 ? "" : "$uid") <<
          "total" << BSON("$sum" << "$kbytes")
        ))));
  
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();

  auto elems = result["result"].Array();
  
  long long total = 0;
  if (!elems.empty())
  {
    try
    {
      total = elems[0]["total"].Long();
    }
    catch (const mongo::DBException& e)
    {
      db::bson::UnserializeFailure("transfers total", e, result, true);
    }
  }
  
  return total;
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
