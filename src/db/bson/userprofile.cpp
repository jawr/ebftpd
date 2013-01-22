#include <vector>
#include <memory>
#include "db/bson/userprofile.hpp"
#include "acl/userprofile.hpp"
#include "acl/types.hpp"
#include "db/bson/bson.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

mongo::BSONObj UserProfile::Serialize(const acl::UserProfile& profile)
{
  mongo::BSONObjBuilder bob;
  bob.append("uid", profile.uid);
  bob.append("creator", profile.creator);
  bob.append("ratio", profile.ratio);
  bob.append("weekly allotment", profile.weeklyAllotment);
  bob.append("home dir", profile.homeDir);
  bob.append("startup dir", profile.startupDir);
  bob.append("idle time", profile.idleTime);
  bob.append("num logins", profile.numLogins);
  bob.append("comment", profile.comment);
  bob.append("max dl speed", profile.maxDlSpeed);
  bob.append("max ul speed", profile.maxUlSpeed);
  bob.append("max sim dl", profile.maxSimDl);
  bob.append("max sim ul", profile.maxSimUl);
  bob.append("logged in", profile.loggedIn);
  
  if (profile.expires) 
    bob.append("expires", ToDateT(*profile.expires));

  if (profile.lastLogin)
    bob.append("last login", ToDateT(*profile.lastLogin));
    
  mongo::BSONArrayBuilder bab;
  for (const auto& kv : profile.sectionRatio)
  {
    bab.append(BSON("section" << kv.first << "ratio" << kv.second));
  }
  
  bob.append("section ratio", bab.arr());
  
  return bob.obj();
}

acl::UserProfile UserProfile::Unserialize(const mongo::BSONObj& bo)
{
  acl::UserProfile profile;

  try
  {  
    profile.uid = bo["uid"].Int();
    profile.ratio = bo["ratio"].Int();
    profile.weeklyAllotment = bo["weekly allotment"].Int();
    profile.homeDir = bo["home dir"].String();
    profile.startupDir = bo["startup dir"].String();
    profile.idleTime = bo["idle time"].Int();
    profile.numLogins = bo["num logins"].Int();
    profile.comment = bo["comment"].String();
    profile.maxDlSpeed = bo["max dl speed"].Int();
    profile.maxUlSpeed = bo["max ul speed"].Int();
    profile.maxSimDl = bo["max sim dl"].Int();
    profile.maxSimUl = bo["max sim ul"].Int();
    profile.creator = bo["creator"].Int();
    profile.loggedIn = bo["logged in"].Int();
    
    if (bo.hasField("expires"))
      profile.expires.reset(ToGregDate(bo["expires"].Date()));
      
    if (bo.hasField("last login"))
      profile.lastLogin.reset(ToPosixTime(bo["last login"].Date()));
    
    mongo::BSONElement oid;
    bo.getObjectID(oid);
    profile.created = ToGregDate(oid.OID().asDateT());
    
    for (const auto& elem : bo["section ratio"].Array())
    {
      profile.sectionRatio.insert(std::make_pair(elem["section"].String(), elem["ratio"].Int()));
    }
    
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("user profile", e, bo);
  }

  return profile;
}
  
} /* bson namespace */
} /* db namespace */
