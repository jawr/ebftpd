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

#include <algorithm>
#include <functional>
#include <cmath>
#include <mongo/client/dbclient.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/stats/stats.hpp"
#include "acl/user.hpp"
#include "stats/date.hpp"
#include "cfg/get.hpp"
#include "util/time.hpp"
#include "db/stats/serialization.hpp"
#include "logs/logs.hpp"
#include "stats/stat.hpp"
#include "db/connection.hpp"
#include "db/serialization.hpp"

namespace db { namespace stats
{

void Update(acl::UserID         uid, 
            long long           kBytes, 
            long long           xfertime, 
            int                 files,
            const std::string&  section, 
            ::stats::Direction  direction, 
            bool                decrement)
{
  if (decrement)
  {
    files *= -1;
    kBytes *= -1;
    xfertime *= -1;
  }

  ::stats::Date date;
  mongo::BSONObjBuilder query;
  query.append("uid", uid);
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
  
  FastConnection conn;
  conn.Update("transfers", query.obj(), update, true);
}

long long XfertimeCorrection(acl::UserID          uid, 
                             long long            kBytes,
                             time_t               modTime, 
                             ::stats::Direction   direction)
{
  long long xfertime = -1; 
  util::Time t(modTime);

  auto cmd = BSON("aggregate" << "transfers" << "pipeline" << 
    BSON_ARRAY(
        BSON("$match" << 
          BSON("year" << t.Year() << "month" << t.Month()  <<
               "week" << t.Week() << "day" << t.Day() <<
               "direction" << util::EnumToString(direction))) <<
        BSON("$group" << 
          BSON("_id" << uid << 
            "total kbytes" << BSON("$sum" << "$kbytes") <<
            "total xfertime" << BSON("$sum" << "$xfertime")
      ))));

  NoErrorConnection conn;
  mongo::BSONObj result;
  if (conn.RunCommand(cmd, result))
  {
    auto elems = result["result"].Array();
    if (!elems.empty())
    {
      try
      {
        long long totalXfertime = elems[0]["total xfertime"].Long();
        if (totalXfertime > 0)
          xfertime = std::ceil(static_cast<double>(totalXfertime) / elems[0]["total kbytes"].Long() * kBytes);
        else
          xfertime = 0;
      }
      catch (const mongo::DBException& e)
      {
        LogException("Unserialize upload decr avg speed", e, result);
      }
    }
  }
  
  return xfertime;
}

void UploadIncr(acl::UserID         uid, 
                long long           kBytes, 
                time_t              modTime, 
                const std::string&  section, 
                int                 files)
{
  if (section.empty()) return; // non stat section not affected by nukes / deleting
  
  long long xfertime = XfertimeCorrection(uid, kBytes, modTime, ::stats::Direction::Upload);
  if (xfertime < 0)
  {
    namespace pt = boost::posix_time;
    logs::Database("Failed to adjust xfertime when incrementing upload stats for date: %1%",
                   pt::to_simple_string(pt::from_time_t(modTime)));
    xfertime = 0;
  }

  Update(uid, kBytes, xfertime, files, "", ::stats::Direction::Upload, true);
  Update(uid, kBytes, xfertime, files, section, ::stats::Direction::Upload, false);
}

void UploadDecr(acl::UserID         uid, 
                long long           kBytes, 
                time_t              modTime, 
                const std::string&  section, 
                int                 files)
{
  if (section.empty()) return; // non stat section not affected by nukes / deleting
  
  long long xfertime = XfertimeCorrection(uid, kBytes, modTime, ::stats::Direction::Upload);
  if (xfertime < 0)
  {
    namespace pt = boost::posix_time;
    logs::Database("Failed to adjust xfertime when decrementing upload stats for date: %1%",
                   pt::to_simple_string(pt::from_time_t(modTime)));
    xfertime = 0;
  }

  Update(uid, kBytes, xfertime, files, section, ::stats::Direction::Upload, true);
  Update(uid, kBytes, xfertime, files, "", ::stats::Direction::Upload, false);
}

void UploadDecr(const acl::User&    user, 
                long long           kBytes, 
                time_t              modTime, 
                const std::string&  section, 
                int                 files)
{
  UploadDecr(user.ID(), kBytes, modTime, section, files);
}

void Upload(const acl::User&    user, 
            long long           kBytes, 
            long long           xfertime, 
            const std::string&  section)
{
  Update(user.ID(), kBytes, xfertime, 1, section, ::stats::Direction::Upload, false);
}

void Download(const acl::User&    user, 
              long long           kBytes, 
              long long           xfertime, 
              const std::string&  section)
{
  Update(user.ID(), kBytes, xfertime, 1, section, ::stats::Direction::Download, false);
}

std::vector< ::stats::Stat> 
RetrieveUsers(const std::string&                    section, 
              ::stats::Timeframe                    timeframe, 
              ::stats::Direction                    direction, 
              boost::optional< ::stats::SortField>  sortField = boost::none, 
              boost::optional<acl::UserID>          uid = boost::none)
{
  static const char* sortFields[] =
  {
    "total kbytes",
    "total files",
    "avg speed"
  };

  mongo::BSONObjBuilder match;
  match.append("direction", util::EnumToString(direction));
  match.appendElements(Serialize(timeframe));
  
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
           
  if (sortField)
  {
    ops.append(BSON("$sort" << BSON(sortFields[static_cast<unsigned>(*sortField)] << -1)));
  }

  auto cmd = BSON("aggregate" << "transfers" << "pipeline" << ops.arr());

  std::vector< ::stats::Stat> users;
  mongo::BSONObj result;
  NoErrorConnection conn;
  if (conn.RunCommand(cmd, result))
  {
    for (const auto& elem : result["result"].Array())
    {
      users.emplace_back(Unserialize(elem.Obj()));
    }
  }
  
  return users;
}

std::vector< ::stats::Stat> 
RetrieveGroups(const std::string&                     section, 
               ::stats::Timeframe                     timeframe, 
               ::stats::Direction                     direction, 
               boost::optional< ::stats::SortField>   sortField = boost::none, 
               boost::optional<acl::GroupID>          gid = boost::none)
{
  auto users = RetrieveUsers(section, timeframe, direction, boost::none);
  std::unordered_map<acl::GroupID, ::stats::Stat> stats;
  
  for (const auto& uStats : users)
  {
    auto user = acl::User::Load(uStats.ID());
    acl::GroupID ugid =  user ? user->PrimaryGID() : acl::GroupID();
    if (gid && ugid != *gid) continue;
    auto it = stats.insert(std::make_pair(ugid, ::stats::Stat(ugid, uStats)));
    if (!it.second) it.first->second.Incr(uStats);
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
      case ::stats::SortField::KBytes  :
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
      groups.emplace_back(kv.second);
    }
  }
  
  return groups;
}

std::vector< ::stats::Stat> 
CalculateUserRanks(const std::string&   section, 
                   ::stats::Timeframe   timeframe, 
                   ::stats::Direction   direction, 
                   ::stats::SortField   sortField)
{
  return RetrieveUsers(section, timeframe, direction, sortField);
}


std::vector< ::stats::Stat>
CalculateGroupRanks(const std::string&  section, 
                    ::stats::Timeframe  timeframe, 
                    ::stats::Direction  direction, 
                    ::stats::SortField  sortField)
{
  return RetrieveGroups(section, timeframe, direction, sortField);
}

::stats::Stat CalculateSingleUser(acl::UserID         uid, 
                                  const std::string&  section, 
                                  ::stats::Timeframe  timeframe, 
                                  ::stats::Direction  direction)
{
  auto users = RetrieveUsers(section, timeframe, direction, boost::none, uid);
  if (users.empty()) return ::stats::Stat(uid);
  return users.front();
}

::stats::Stat CalculateSingleGroup(acl::GroupID         gid, 
                                   const std::string&   section, 
                                   ::stats::Timeframe   timeframe, 
                                   ::stats::Direction   direction)
{
  auto groups = RetrieveGroups(section, timeframe, direction, boost::none, gid);
  if (groups.empty()) return ::stats::Stat(gid);
  return groups.front();
}

} /* stats namespace */
} /* db namespace */
