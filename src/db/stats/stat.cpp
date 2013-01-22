#include <cstdint>
#include <boost/thread/future.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <mongo/client/dbclient.h>
#include "db/stats/stat.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "db/bson/stat.hpp"
#include "util/error.hpp"
#include "util/time.hpp"
#include "logs/logs.hpp"
#include "db/bson/error.hpp"

namespace db { namespace stats
{

void Get(mongo::Query& query, QueryResults& results)
{
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("transfers", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.empty()) throw util::RuntimeError("No results");
  
  return;
}

mongo::BSONObj GetFromCommand(const mongo::BSONObj& match)
{
  boost::unique_future<bool> future;
  mongo::BSONObj cmd = BSON("aggregate" << "transfers" << "pipeline" <<
    BSON_ARRAY(BSON("$match" << match) << BSON("$group" <<
      BSON("_id" << "$uid" << "files" << BSON("$sum" << "$files") << "bytes" <<
      BSON("$sum" << "$bytes") << "xfertime" << BSON("$sum" << "$xfertime")))
    ));
  mongo::BSONObj ret;
  TaskPtr task(new db::RunCommand(cmd, ret, future));
  Pool::Queue(task);

  future.wait();

  return ret;      
}

::stats::Stat GetWeekDown(acl::UserID uid, int week, int year)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid << "week" << week << "year" << year
    << "direction" << "dn");
  Get(query, results);
  return db::bson::Stat::Unserialize(results.front());
}

std::unordered_map<acl::UserID, ::stats::Stat> GetAllUp(const std::vector<acl::User>& users)
{
  std::unordered_map<acl::UserID, ::stats::Stat> stats;
  for (auto& user: users)
  {
    stats.insert(std::make_pair(user.UID(), GetAllUp(user)));
  } 
  return stats; 
}

std::unordered_map<acl::UserID, ::stats::Stat> GetAllDown(const std::vector<acl::User>& users)
{
  std::unordered_map<acl::UserID, ::stats::Stat> stats;
  for (auto& user: users)
  {
    stats.insert(std::make_pair(user.UID(), GetAllDown(user)));
  } 
  return stats; 
}

::stats::Stat GetAllDown(const acl::User& user)
{
  mongo::BSONObj match = BSON("uid" << user.UID() << "direction" << "dn");
  mongo::BSONObj ret = GetFromCommand(match);
  return db::bson::Stat::UnserializeRaw(ret);

}

::stats::Stat GetAllUp(const acl::User& user)
{
  mongo::BSONObj match = BSON("uid" << user.UID() << "direction" << "up");
  mongo::BSONObj ret = GetFromCommand(match);
  return db::bson::Stat::UnserializeRaw(ret);

}

::stats::Stat GetWeekUp(acl::UserID uid, int week, int year)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid << "week" << week << "year" << year
    << "direction" << "up");
  Get(query, results);
  return db::bson::Stat::Unserialize(results.front());
}

void UploadDecr(const acl::User& user, long long bytes, time_t modTime, const std::string& section)
{
  util::Time t(modTime);

  auto cmd = BSON("aggregate" << "transfers" << "pipeline" << 
    BSON_ARRAY(
        BSON("$match" << 
          BSON("year" << t.Year() << "month" << t.Month()  <<
               "week" << t.Week() << "day" << t.Day())) <<
        BSON("$group" << 
          BSON("_id" << user.UID() << 
            "total bytes" << BSON("$sum" << "$bytes") <<
            "total xfertime" << BSON("$sum" << "$xfertime")
      ))));
    
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();
  
  double speed = 0;
  try
  {
    long long xfertime = result["total xfertime"].Long();
    if (xfertime > 0)
      speed = result["total bytes"].Long() / xfertime / 1.0;
    else
      speed = result["total bytes"].Long() / 1.0;
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("upload decr average speed", e, result, true);
  }

  assert(!section.empty());
  long long xfertime = bytes / speed;
  Upload(user, bytes, xfertime, section, true);
  Upload(user, bytes, xfertime, "");
}

void Upload(const acl::User& user, long long bytes, long long xfertime, 
    const std::string& section, bool decrement)
{
  int files = 1;
  
  if (decrement)
  {
    files *= -1;
    bytes *= -1;
    xfertime *= -1;
  }

  util::Time time;
  mongo::Query query = QUERY("uid" << user.UID() << "day" << time.Day()
    << "week" << time.Week() << "month" 
    << time.Month() << "year" << time.Year()
    << "direction" << "up"
    << "section" << section);
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << files) <<
    "$inc" << BSON("bytes" << bytes) <<
    "$inc" << BSON("xfertime" << xfertime));
    
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Download(const acl::User& user, long long bytes, long long xfertime, const std::string& section)
{
  util::Time time;
  mongo::Query query = QUERY("uid" << user.UID() << "day" << time.Day()
    << "week" << time.Week() << "month" 
    << time.Month() << "year" << time.Year()
    << "direction" << "dn"
    << "section" << section);
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("bytes" << bytes) <<
    "$inc" << BSON("xfertime" << xfertime)); // how to handle the xfertime
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

// end
}
}
