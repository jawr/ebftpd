#include "db/bson/groupprofile.hpp"
#include "acl/groupprofile.hpp"
#include "logs/logs.hpp"
#include "db/exception.hpp"

namespace db { namespace bson
{

mongo::BSONObj GroupProfile::Serialize(const acl::GroupProfile& profile)
{
  mongo::BSONObjBuilder bob;
  bob.append("gid", profile.gid);
  bob.append("description", profile.description);
  bob.append("slots", profile.slots);
  bob.append("leech slots", profile.leechSlots);
  bob.append("allotment slots", profile.allotSlots);
  bob.append("max allotment slots", profile.maxAllotSlots);
  bob.append("max logins", profile.maxLogins);
  bob.append("comment", profile.comment);
  return bob.obj();
}

acl::GroupProfile GroupProfile::Unserialize(const mongo::BSONObj& bo)
{
  try
  {
    acl::GroupProfile profile(bo["gid"].Int());
    profile.description = bo["description"].String();
    profile.comment = bo["comment"].String();
    profile.slots = bo["slots"].Int();
    profile.leechSlots = bo["leech slots"].Int();
    profile.allotSlots = bo["allotment slots"].Int();
    profile.maxAllotSlots = bo["max allotment slots"].Int();
    profile.slots = bo["loginss"].Int();
    profile.slots = bo["max logins"].Int();
    return profile;
  }
  catch (const mongo::DBException& e)
  {
    logs::db << "Error while unserialising group profile: " << e.what() << logs::endl;
    throw db::DBError("Unable to load group profile.");
  }
}

} /* bson namespace */
} /* db namespace */
