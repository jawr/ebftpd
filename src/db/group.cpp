#include <mongo/client/dbclient.h>
#include "db/group.hpp"
#include "acl/group.hpp"
#include "util/verify.hpp"
#include "db/serialization.hpp"
#include "db/connection.hpp"
#include "db/error.hpp"

namespace db
{

acl::GroupID Group::Create()
{
  db::SafeConnection conn;
  return conn.InsertAutoIncrement("groups", group, "gid");
}

void Group::SaveField(const std::string& field)
{
  db::NoErrorConnection conn;
  conn.SetField("groups", QUERY("gid" << group.ID()), group, { field, "modified" });
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

mongo::BSONObj Serialize(const acl::Group& group)
{
  mongo::BSONObjBuilder bob;
  bob.append("modified", ToDateT(group.modified));
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

acl::Group Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    acl::Group group;
    group.modified = ToPosixTime(obj["modified"].Date());
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

boost::optional<acl::Group> Load(acl::GroupID gid)
{
  NoErrorConnection conn;
  return conn.QueryOne<acl::Group>("groups", QUERY("gid" << gid));
}

namespace
{

std::shared_ptr<GroupCache> cache;

struct GroupPair
{
  std::string name;
  acl::GroupID gid;
};

GroupPair Unserialize(const mongo::BSONObj& obj)
{
  GroupPair pair;
  pair.name = obj["name"].String();
  pair.gid = obj["gid"].Int();
  return pair;
}

std::string LookupNameByGID(acl::GroupID gid)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<GroupPair>("groups", QUERY("gid" << gid), &fields);
  if (!pair) return "unknown";
  return pair->name;
}

acl::GroupID LookupGIDByName(const std::string& name)
{
  NoErrorConnection conn;  
  auto fields = BSON("gid" << 1 << "name" << 1);
  auto pair = conn.QueryOne<GroupPair>("groups", QUERY("name" << name), &fields);
  if (!pair) return -1;
  return pair->gid;
}

} /* unnamed namespace */

std::string GIDToName(acl::GroupID gid)
{
  if (cache) return cache->GIDToName(gid);
  return LookupNameByGID(gid);
}

acl::GroupID NameToGID(const std::string& name)
{
  if (cache) return cache->NameToGID(name);
  return LookupGIDByName(name);
}

} /* acl namespace */
