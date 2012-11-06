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

std::map<acl::UserID, ::stats::Stat> GetAllUp(const std::vector<acl::User>& users)
{
  std::map<acl::UserID, ::stats::Stat> stats;
  for (auto& user: users)
  {
    mongo::BSONObj match = BSON("uid" << user.UID() << "direction" << "up");  
    mongo::BSONObj ret = GetFromCommand(match);
    stats.insert(std::make_pair(user.UID(), 
      db::bson::Stat::UnserializeRaw(ret)));
  } 
  return stats; 
}

std::map<acl::UserID, ::stats::Stat> GetAllDown(const std::vector<acl::User>& users)
{
  std::map<acl::UserID, ::stats::Stat> stats;
  for (auto& user: users)
  {
    mongo::BSONObj match = BSON("uid" << user.UID() << "direction" << "dn");
    mongo::BSONObj ret = GetFromCommand(match);
    stats.insert(std::make_pair(user.UID(), 
      db::bson::Stat::UnserializeRaw(ret)));
  } 
  return stats; 
}

::stats::Stat GetWeekUp(acl::UserID uid, int week, int year)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid << "week" << week << "year" << year
    << "direction" << "up");
  Get(query, results);
  return db::bson::Stat::Unserialize(results.front());
}

void UploadDecr(const acl::User& user, long long bytes)
{
  util::Time time;
  mongo::Query query = QUERY("uid" << user.UID() << "day" << time.Day()
    << "week" << time.Week() << "month" 
    << time.Month() << "year" << time.Year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << -1) <<
    "$inc" << BSON("bytes" << bytes*-1) <<
    "$inc" << BSON("xfertime" << static_cast<long long>(0)));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Upload(const acl::User& user, long long bytes, long long xfertime)
{
  util::Time time;
  mongo::Query query = QUERY("uid" << user.UID() << "day" << time.Day()
    << "week" << time.Week() << "month" 
    << time.Month() << "year" << time.Year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("bytes" << bytes) <<
    "$inc" << BSON("xfertime" << xfertime));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Download(const acl::User& user, long long bytes, long long xfertime)
{
  logs::debug << "DOWNLOAD: " << bytes << logs::endl;
  util::Time time;
  mongo::Query query = QUERY("uid" << user.UID() << "day" << time.Day()
    << "week" << time.Week() << "month" 
    << time.Month() << "year" << time.Year()
    << "direction" << "dn");
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
