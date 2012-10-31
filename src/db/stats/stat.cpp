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

::stats::Stat GetWeekDown(const acl::UserID& uid, int week, int year)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid << "week" << week << "year" << year
    << "direction" << "dn");
  Get(query, results);
  return db::bson::Stat::Unserialize(results.front());
}

void GetAllDown(const std::vector<acl::User>& users,
  std::map<acl::UserID, ::stats::Stat>& stats, int week, int year)
{
  
}

::stats::Stat GetWeekUp(const acl::UserID& uid, int week, int year)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid << "week" << week << "year" << year
    << "direction" << "up");
  Get(query, results);
  return db::bson::Stat::Unserialize(results.front());
}

void UploadDecr(const acl::User& user, long long kbytes)
{
  util::Time::Update();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << util::Time::Day()
    << "week" << util::Time::Week() << "month" 
    << util::Time::Month() << "year" << util::Time::Year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << -1) <<
    "$inc" << BSON("kbytes" << kbytes*-1) <<
    "$inc" << BSON("xfertime" << (long long)0));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Upload(const acl::User& user, long long kbytes, long long xfertime)
{
  util::Time::Update();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << util::Time::Day()
    << "week" << util::Time::Week() << "month" 
    << util::Time::Month() << "year" << util::Time::Year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("kbytes" << kbytes) <<
    "$inc" << BSON("xfertime" << xfertime));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Download(const acl::User& user, long long kbytes, long long xfertime)
{
  util::Time::Update();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << util::Time::Day()
    << "week" << util::Time::Week() << "month" 
    << util::Time::Month() << "year" << util::Time::Year()
    << "direction" << "dn");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("kbytes" << kbytes) <<
    "$inc" << BSON("xfertime" << xfertime)); // how to handle the xfertime
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

// end
}
}
