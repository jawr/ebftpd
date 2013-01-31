#include <algorithm>
#include <functional>
#include <mongo/client/dbclient.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/stats/stat.hpp"
#include "acl/user.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"
#include "stats/date.hpp"
#include "cfg/get.hpp"
#include "util/time.hpp"
#include "db/bson/error.hpp"
#include "db/bson/timeframe.hpp"
#include "db/bson/stat.hpp"
#include "acl/usercache.hpp"

namespace db { namespace stats
{

void Update(const acl::User& user, long long kBytes, long long xfertime, 
    const std::string& section, ::stats::Direction direction, bool decrement)
{
  int files = 1;
  if (decrement)
  {
    files *= -1;
    kBytes *= -1;
    xfertime *= -1;
  }

  ::stats::Date date(cfg::Get().WeekStart() == cfg::WeekStart::Monday);
  mongo::BSONObjBuilder query;
  query.append("uid", user.UID());
  query.append("day", date.Day());
  query.append("week", date.Week());
  query.append("month", date.Month());
  query.append("year", date.Year());
  query.append("direction", util::EnumToString(direction));
  query.append("section", section);

  mongo::BSONObj update = BSON(
    "$inc" << BSON("files" << files) <<
    "$inc" << BSON("kbytes" << kBytes) <<
    "$inc" << BSON("xfertime" << xfertime));
    
  TaskPtr task(new db::Update("transfers", query.obj(), update, true));
  Pool::Queue(task);
}

void UploadDecr(const acl::User& user, long long kBytes, time_t modTime, const std::string& section)
{
  util::Time t(modTime);

  auto cmd = BSON("aggregate" << "transfers" << "pipeline" << 
    BSON_ARRAY(
        BSON("$match" << 
          BSON("year" << t.Year() << "month" << t.Month()  <<
               "week" << t.Week() << "day" << t.Day())) <<
        BSON("$group" << 
          BSON("_id" << user.UID() << 
            "total kbytes" << BSON("$sum" << "$kbytes") <<
            "total xfertime" << BSON("$sum" << "$xfertime")
      ))));
    
  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();

  result = result["result"].Obj();
  
  long long xfertime = 0;
  if (result.nFields() > 0)
  {
    try
    {
      long long totalXfertime = result[0]["total xfertime"].Long();
      if (totalXfertime > 0)
        xfertime = kBytes / result[0]["total kbytes"].Long() / totalXfertime;
      else
        xfertime = kBytes / result[0]["total kbytes"].Long();
    }
    catch (const mongo::DBException& e)
    {
      db::bson::UnserializeFailure("upload decr average speed", e, result, true);
    }
  }
  else
  {
    namespace pt = boost::posix_time;
    logs::db << "Failed to adjust xfertime on file deletion, "
             << "no data available for that date: " 
             << pt::to_simple_string(pt::from_time_t(modTime)) << logs::endl;
  }

  assert(!section.empty());
  Update(user, kBytes, xfertime, section, ::stats::Direction::Upload, true);
  Update(user, kBytes, xfertime, "", ::stats::Direction::Upload, false);
}

void Upload(const acl::User& user, long long kBytes, long long xfertime, const std::string& section)
{
  Update(user, kBytes, xfertime, section, ::stats::Direction::Upload, false);
}

void Download(const acl::User& user, long long kBytes, long long xfertime, const std::string& section)
{
  Update(user, kBytes, xfertime, section, ::stats::Direction::Download, false);
}

std::vector< ::stats::Stat> RetrieveUsers(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      boost::optional< ::stats::SortField> sortField = boost::none, 
      boost::optional<acl::UserID> uid = boost::none)
{
  static const char* sortFields[] =
  {
    "total kbytes",
    "total files",
    "avg speed"
  };

  mongo::BSONObjBuilder match;
  match.append("direction", util::EnumToString(direction));
  match.appendElements(::db::bson::TimeframeSerialize(timeframe));
  
  if (!section.empty())
    match.append("section", section);
  else
  {
    mongo::BSONArrayBuilder sections;
    for (const auto& kv : cfg::Get().Sections())
      sections.append(kv.first);
    match.appendElements(BSON("section" << BSON("$in" << sections.arr())));
  }
  
  if (uid) match.append("uid", *uid);
  
  mongo::BSONArrayBuilder ops;
  ops.append(BSON("$match" << match.obj()));
  ops.append(BSON("$group" << BSON("_id" << "$uid" <<
             "total kbytes" << BSON("$sum" << "$kbytes") <<
             "total files" << BSON("$sum" << "$files") <<
             "total xfertime" << BSON("$sum" << "$xfertime"))));
  
  ops.append(BSON("$project" << BSON("total kbytes" << 1 <<
             "total files" << 1 <<
             "total xfertime" << 1 <<
             "avg speed" << BSON("$divide" << 
             BSON_ARRAY("$total kbytes" << "$total xfertime")))));
           
  if (sortField) ops.append(BSON("$sort" << BSON(sortFields[static_cast<unsigned>(*sortField)] << -1)));

  auto cmd = BSON("aggregate" << "transfers" << "pipeline" << ops.arr());

  boost::unique_future<bool> future;
  mongo::BSONObj result;
  TaskPtr task(new db::RunCommand(cmd, result, future));
  Pool::Queue(task);
  future.wait();

  auto elems = result["result"].Array();

  std::vector< ::stats::Stat> users;
  for (const auto& elem : elems)
  {
    users.push_back(bson::Stat::Unserialize(elem.Obj()));
  }
  
  return users;
}

std::vector< ::stats::Stat> RetrieveGroups(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      boost::optional< ::stats::SortField> sortField = boost::none, 
      boost::optional<acl::GroupID> gid = boost::none)
{
  auto users = RetrieveUsers(section, timeframe, direction, boost::none);
  std::unordered_map<acl::GroupID, ::stats::Stat> stats;
  
  for (const auto& user : users)
  {
    auto ugid = acl::UserCache::PrimaryGID(user.ID());
    if (gid && ugid != *gid) continue;
    auto it = stats.insert(std::make_pair(ugid, ::stats::Stat(ugid, user)));
    if (!it.second) it.first->second.Incr(user);
  }

  std::vector< ::stats::Stat> groups;
  groups.reserve(stats.size());
  
  if (sortField)
  {
    std::function<bool(const ::stats::Stat& s1, const ::stats::Stat& s2)> sortCompare;
    
    switch (*sortField)
    {
      case ::stats::SortField::Files  :
        sortCompare = [&](const ::stats::Stat& s1, const ::stats::Stat& s2)
                      {
                        return s1.Files() > s2.Files();
                      };
        break;
      case ::stats::SortField::Bytes  :
        sortCompare = [&](const ::stats::Stat& s1, const ::stats::Stat& s2)
                      {
                        return s1.KBytes() > s2.KBytes();
                      };
        break;
      case ::stats::SortField::Speed  :
        sortCompare = [&](const ::stats::Stat& s1, const ::stats::Stat& s2)
                      {
                        return s1.Speed() > s2.Speed();
                      };
        break;
    }
    
    for (const auto& kv : stats)
    {
      auto pos = std::lower_bound(groups.begin(), groups.end(), kv.second, sortCompare);
      groups.insert(pos, kv.second);
    }
  }
  else
  {
    for (const auto& kv : stats)
    {
      groups.push_back(kv.second);
    }
  }
  
  return groups;
}

std::vector< ::stats::Stat> CalculateUserRanks(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      ::stats::SortField sortField)
{
  return RetrieveUsers(section, timeframe, direction, sortField);
}


std::vector< ::stats::Stat> CalculateGroupRanks(
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction, 
      ::stats::SortField sortField)
{
  return RetrieveGroups(section, timeframe, direction, sortField);
}

::stats::Stat CalculateSingleUser(
      acl::UserID uid, 
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction)
{
  auto users = RetrieveUsers(section, timeframe, direction, boost::none, uid);
  if (users.empty()) return ::stats::Stat(uid);
  return users.front();
}

::stats::Stat CalculateSingleGroup(
      acl::GroupID gid, 
      const std::string& section, 
      ::stats::Timeframe timeframe, 
      ::stats::Direction direction)
{
  auto groups = RetrieveGroups(section, timeframe, direction, boost::none, gid);
  if (groups.empty()) return ::stats::Stat(gid);
  return groups.front();
}

}
}
