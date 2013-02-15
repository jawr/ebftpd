#include <mongo/client/dbclient.h>
#include "db/groupprofile.hpp"
#include "recode/acl/groupprofile.hpp"
#include "util/verify.hpp"
#include "db/serialization.hpp"
#include "db/connection.hpp"
#include "db/error.hpp"

namespace db
{

void GroupProfile::Save()
{
  db::SafeConnection conn;
  conn.InsertOne("users", profile);
}

void GroupProfile::SaveField(const std::string& field)
{
  db::SafeConnection conn;
  conn.SetField("groups", QUERY("gid" << profile.ID()), profile, { field, "modified" });
}

void GroupProfile::SaveDescription()
{
  SaveField("description");  
}

void GroupProfile::SaveComment()
{
  SaveField("comment");
}

void GroupProfile::SaveSlots()
{
  SaveField("slots");
}

void GroupProfile::SaveLeechSlots()
{
  SaveField("leech slots");
}

void GroupProfile::SaveAllotmentSlots()
{
  SaveField("allotment slots");
}

void GroupProfile::SaveMaxAllotmentSize()
{
  SaveField("max allotment size");
}

void GroupProfile::SaveMaxLogins()
{
  SaveField("max logins");
}

mongo::BSONObj Serialize(const acl::GroupProfile& profile)
{
  mongo::BSONObjBuilder bob;
  bob.append("modified", ToDateT(profile.modified));
  bob.append("name", profile.name);
  bob.append("gid", profile.id);
  bob.append("description", profile.description);
  bob.append("slots", profile.slots);
  bob.append("leech slots", profile.leechSlots);
  bob.append("allotment slots", profile.allotmentSlots);
  bob.append("max allotment size", profile.maxAllotmentSize);
  bob.append("max logins", profile.maxLogins);
  bob.append("comment", profile.comment);
  return bob.obj();
}

acl::GroupProfile Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    acl::GroupProfile profile;
    profile.modified = ToPosixTime(obj["modified"].Date());
    profile.id = obj["gid"].Int();
    profile.name = obj["name"].String();
    profile.description = obj["description"].String();
    profile.comment = obj["comment"].String();
    profile.slots = obj["slots"].Int();
    profile.leechSlots = obj["leech slots"].Int();
    profile.allotmentSlots = obj["allotment slots"].Int();
    profile.maxAllotmentSize = obj["max allotment size"].Long();
    profile.slots = obj["slots"].Int();
    profile.maxLogins = obj["max logins"].Int();
    return profile;
  }
  catch (const mongo::DBException& e)
  {
    LogException("Unserialize group profile", e, obj);
    throw e;
  }
}

boost::optional<acl::GroupProfile> Load(acl::GroupID gid)
{
  NoErrorConnection conn;
  return conn.QueryOne<acl::GroupProfile>("groups", QUERY("gid" << gid));
}

} /* acl namespace */
