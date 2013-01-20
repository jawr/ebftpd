#include <boost/optional.hpp>
#include "db/bson/groupprofile.hpp"
#include "acl/groupprofile.hpp"
#include "db/bson/error.hpp"

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
  bob.append("allotment size", profile.allotSize);
  bob.append("max logins", profile.maxLogins);
  bob.append("comment", profile.comment);
  return bob.obj();
}

acl::GroupProfile GroupProfile::Unserialize(const mongo::BSONObj& bo)
{
  boost::optional<acl::GroupProfile> profile;
  try
  {
    profile->gid = bo["gid"].Int();
    profile->description = bo["description"].String();
    profile->comment = bo["comment"].String();
    profile->slots = bo["slots"].Int();
    profile->leechSlots = bo["leech slots"].Int();
    profile->allotSlots = bo["allotment slots"].Int();
    profile->allotSize = bo["allotment size"].Int();
    profile->slots = bo["loginss"].Int();
    profile->slots = bo["max logins"].Int();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("group profile", e, bo);
  }
  
  return *profile;
}

} /* bson namespace */
} /* db namespace */
