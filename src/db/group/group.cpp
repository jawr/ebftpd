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

#include <boost/optional.hpp>
#include <mongo/client/dbclient.h>
#include "util/string.hpp"
#include "db/group/group.hpp"
#include "acl/group.hpp"
#include "util/verify.hpp"
#include "db/serialization.hpp"
#include "db/connection.hpp"
#include "db/error.hpp"
#include "acl/groupdata.hpp"

namespace db
{


template <> mongo::BSONObj Serialize<acl::GroupData>(const acl::GroupData& group)
{
  mongo::BSONObjBuilder bob;
  bob.append("name", group.name);
  bob.append("gid", group.id);
  bob.append("description", group.description);
  bob.append("slots", group.slots);
  bob.append("leech slots", group.leechSlots);
  bob.append("allotment slots", group.allotmentSlots);
  bob.append("max allotment size", group.maxAllotmentSize);
  bob.append("max logins", group.maxLogins);
  bob.append("comment", group.comment);
  return bob.obj();
}

template <> acl::GroupData Unserialize<acl::GroupData>(const mongo::BSONObj& obj)
{
  try
  {
    acl::GroupData group;
    group.id = obj["gid"].Int();
    group.name = obj["name"].String();
    group.description = obj["description"].String();
    group.comment = obj["comment"].String();
    group.slots = obj["slots"].Int();
    group.leechSlots = obj["leech slots"].Int();
    group.allotmentSlots = obj["allotment slots"].Int();
    group.maxAllotmentSize = obj["max allotment size"].Long();
    group.slots = obj["slots"].Int();
    group.maxLogins = obj["max logins"].Int();
    return group;
  }
  catch (const mongo::DBException& e)
  {
    LogException("Unserialize group", e, obj);
    throw e;
  }
}

bool Group::Create()
{
  NoErrorConnection conn;
  group.id = conn.InsertAutoIncrement("groups", group, "gid");
  if (group.id == -1) return false;
  UpdateLog();
  return true;
}

void Group::UpdateLog() const
{
  FastConnection conn;
  auto entry = BSON("collection" << "groups" << "id" << group.id);
  conn.Insert("updatelog", entry);
}


void Group::SaveField(const std::string& field)
{
  NoErrorConnection conn;

  conn.SetField("groups", QUERY("gid" << group.id), group, field);
  UpdateLog();
}

bool Group::SaveName()
{
  try
  {
    SafeConnection conn;
    conn.SetField("groups", QUERY("gid" << group.id), group, "name");
    UpdateLog();
    return true;
  }
  catch (const DBError&)
  {
    return false;
  }
}

void Group::SaveDescription()
{
  SaveField("description");  
}

void Group::SaveComment()
{
  SaveField("comment");
}

void Group::SaveSlots()
{
  SaveField("slots");
}

void Group::SaveLeechSlots()
{
  SaveField("leech slots");
}

void Group::SaveAllotmentSlots()
{
  SaveField("allotment slots");
}

void Group::SaveMaxAllotmentSize()
{
  SaveField("max allotment size");
}

void Group::SaveMaxLogins()
{
  SaveField("max logins");
}

int Group::NumSlotsUsed() const
{
  NoErrorConnection conn;
  mongo::BSONObjBuilder bob;
  bob.append("primary gid", group.id);
  bob.appendRegex("flags", "^[^6]*$");
  return conn.Count("users", bob.obj());  
}

int Group::NumMembers() const
{
  mongo::BSONArrayBuilder bab;
  bab.append(group.id);
  auto query = BSON("$or" << BSON_ARRAY(BSON("primary gid" << group.id) <<
                                        BSON("secondary gids" << BSON("$in" << bab.arr()))));
  NoErrorConnection conn;
  return conn.Count("users", query);
}

int Group::NumLeeches() const
{
  NoErrorConnection conn;
  auto query = BSON("primary gid" << group.id <<
                    "ratio" << BSON("$elemMatch" << BSON("section" << "" << 
                                                         "value" << 0)));
  return conn.Count("users", query);
}

int Group::NumAllotments() const
{
  NoErrorConnection conn;
  auto query = BSON("primary gid" << group.id <<
                    "weekly allotment" << BSON("$elemMatch" << BSON("section" << "" << 
                                                                    "value" << BSON("$gt" << 0))));
  return conn.Count("users", query);
}

long long Group::TotalAllotmentSize() const
{
  NoErrorConnection conn;
  auto cmd = BSON("aggregate" << "users" << "pipeline" <<
    BSON_ARRAY(
      BSON("$unwind" << "$weekly allotment") <<
      BSON("$match" << 
        BSON("primary gid" << group.id <<
             "weekly allotment.section" << "")
      ) <<
      BSON("$group" << 
        BSON("_id" << "" <<
             "total" << BSON("$sum" << "$weekly allotment.value"))
     )));

  mongo::BSONObj result;
  if (conn.RunCommand(cmd, result))
  {
    auto elems = result["result"].Array();
    if (!elems.empty())
    {
      try
      {
        return elems[0]["total"].Long();
      }
      catch (const mongo::DBException& e)
      {
        LogException("Unserialize allotment size total", e, result);
      }
    }
  }
  
  return -1;
}

void Group::Purge() const
{
  NoErrorConnection conn;
  conn.Remove("groups", QUERY("gid" << group.id));
  UpdateLog();
}

boost::optional<acl::GroupData> Group::Load(acl::GroupID gid)
{
  NoErrorConnection conn;
  return conn.QueryOne<acl::GroupData>("groups", QUERY("gid" << gid));
}

boost::optional<acl::GroupData> Group::Load(const std::string& name)
{
  NoErrorConnection conn;
  return conn.QueryOne<acl::GroupData>("groups", QUERY("name" << name));
}

namespace
{

template <typename T>
std::vector<T> GetGroupsGeneric(const std::string& multiStr, const mongo::BSONObj* fields)
{
  std::vector<std::string> toks;
  util::Split(toks, multiStr, " ", true);
  
  mongo::Query query;
  if (std::find(toks.begin(), toks.end(), "*") == toks.end())
  {
    mongo::BSONArrayBuilder namesBab;
    
    for (std::string tok : toks)
    {
      if (tok[0] == '=') tok.erase(0, 1);
      namesBab.append(tok);
    }
    
    query = QUERY("name" << BSON("$in" << namesBab.arr()));
  }
  
  NoErrorConnection conn;
  return conn.QueryMulti<T>("groups", query, 0, 0, fields);
}

}

std::vector<acl::GroupID> GetGIDs(const std::string& multiStr)
{
  auto fields = BSON("gid" << 1);
  return GetGroupsGeneric<acl::GroupID>(multiStr, &fields);
}

std::vector<acl::GroupData> GetGroups(const std::string& multiStr)
{
  return GetGroupsGeneric<acl::GroupData>(multiStr, nullptr);
}

} /* acl namespace */
