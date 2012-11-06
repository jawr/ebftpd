#include <vector>
#include <memory>
#include "db/bson/userprofile.hpp"
#include "acl/userprofile.hpp"
#include "acl/types.hpp"
#include "db/bson/bson.hpp"

namespace db { namespace bson
{

mongo::BSONObj UserProfile::Serialize(const acl::UserProfile& profile)
{
  mongo::BSONObjBuilder bob;
  bob.append("uid", profile.uid);
  bob.append("ratio", profile.ratio);
  bob.append("weekly allotment", profile.weeklyAllotment);
  bob.append("home dir", profile.homeDir);
  bob.append("startup dir", profile.startupDir);
  bob.append("idle time", profile.idleTime);
  bob.append("expires", profile.Expires());
  bob.append("num logins", profile.numLogins);
  bob.append("tagline", profile.tagline);
  bob.append("comment", profile.comment);
  bob.append("max dl speed", profile.maxDlSpeed);
  bob.append("max ul speed", profile.maxUlSpeed);
  bob.append("max sim dl", profile.maxSimDl);
  bob.append("max sim ul", profile.maxSimUl);
  bob.append("creator", profile.creator);
  return bob.obj();
}

acl::UserProfile UserProfile::Unserialize(const mongo::BSONObj& bo)
{
  acl::UserProfile profile(bo["uid"].Int(), bo["creator"].Int());

  profile.ratio = bo["ratio"].Int();
  profile.weeklyAllotment = bo["weekly allotment"].Int();
  profile.homeDir = bo["home dir"].String();
  profile.startupDir = bo["startup dir"].String();
  profile.idleTime = bo["idle time"].Int();
  profile.SetExpires(bo["expires"].String());
  profile.numLogins = bo["num logins"].Int();
  profile.tagline = bo["tagline"].String();
  profile.comment = bo["comment"].String();
  profile.maxDlSpeed = bo["max dl speed"].Int();
  profile.maxUlSpeed = bo["max ul speed"].Int();
  profile.maxSimDl = bo["max sim dl"].Int();
  profile.maxSimUl = bo["max sim ul"].Int();
  profile.creator = bo["creator"].Int();

  if (bo.hasField("last login"))
    profile.lastLogin = bo["last login"].Date().toString();

  if (bo.hasField("logged in"))
    profile.loggedIn = bo["logged in"].Int();
  
  mongo::BSONElement date;
  bo.getObjectID(date);
  
  struct tm created;
  date.OID().asDateT().toTm(&created);
  profile.created = boost::gregorian::date_from_tm(created);

  return profile;
}
  
} /* bson namespace */
} /* db namespace */
