#include "db/userprofile.hpp"
#include "db/connection.hpp"
#include "recode/acl/userprofile.hpp"
#include "db/serialization.hpp"
#include "db/error.hpp"

namespace db
{

void UserProfile::Save()
{
  db::SafeConnection conn;
  conn.InsertOne("users", profile);
}

void UserProfile::SaveField(const std::string& field)
{
  db::SafeConnection conn;
  conn.SetField("users", QUERY("uid" << profile.ID()), profile, { field, "modified" });
}

void UserProfile::SaveName()
{
  SaveField("name");
}

void UserProfile::SaveIPMasks()
{
  SaveField("ip masks");
}

void UserProfile::SavePassword()
{
  SaveField("password");
}

void UserProfile::SaveFlags()
{
  SaveField("flags");
}

void UserProfile::SavePrimaryGID()
{
  SaveField("primary gid");
}

void UserProfile::SaveSecondaryGIDs()
{
  SaveField("secondary gids");
}

void UserProfile::SaveGadminGIDs()
{
  SaveField("gadmin gids");
}

void UserProfile::SaveWeeklyAllotment()
{
  SaveField("weekly allotment");
}

void UserProfile::SaveHomeDir()
{
  SaveField("home dir");
}

void UserProfile::SaveIdleTime()
{
  SaveField("idle time");
}

void UserProfile::SaveExpires()
{
  SaveField("expires");
}

void UserProfile::SaveNumLogins()
{
  SaveField("num logins");
}

void UserProfile::SaveComment()
{
  SaveField("comment");
}

void UserProfile::SaveTagline()
{
  SaveField("tagline");
}

void UserProfile::SaveMaxDownSpeed()
{
  SaveField("max down speed");
}

void UserProfile::SaveMaxUpSpeed()
{
  SaveField("max up speed");
}

void UserProfile::SaveMaxSimDown()
{
  SaveField("max sim down");
}

void UserProfile::SaveMaxSimUp()
{
  SaveField("max sim up");
}

void UserProfile::SaveLoggedIn()
{
  SaveField("logged in");
}

void UserProfile::SaveLastLogin()
{
  SaveField("last login");
}

void UserProfile::SaveRatio()
{
  SaveField("ratio");
}

void UserProfile::IncrCredits(const std::string& section, long long kBytes, 
                 long long& newCredits)
{
  // in the old thread pool design this used be done in a separate thread
  // this may need to be changed to be async again.
  db::FastConnection conn;
  
  auto doIncrement = [&]()
    {
      auto updateExisting = [&]() -> bool
        {
          auto query = BSON("uid" << profile.ID() << 
                            "credits" << BSON("$elemMatch" << BSON("section" << section)));
                            
          auto update = BSON("$inc" << BSON("credits.$.value" << kBytes));
                            
          auto cmd = BSON("findandmodify" << "users" <<
                          "query" << query <<
                          "update" << update);
                          
          mongo::BSONObj result;
          return conn.RunCommand(cmd, result) && 
                 result["value"].type() != mongo::jstNULL;
        };

      auto doInsert = [&]() -> bool
      {
        auto query = QUERY("uid" << profile.ID() << "credits" << BSON("$not" << 
                           BSON("$elemMatch" << BSON("section" << section))));
        auto update = BSON("$push" << BSON("credits" << BSON("section" << section << "value" << kBytes)));
        return conn.Update("users", query, update, false) > 0;
      };
      
      if (updateExisting()) return;
      if (doInsert()) return;
      if (updateExisting()) return;

      logs::db << "Unable to increment credits for UID " << profile.ID();
      if (!section.empty()) logs::db << " in section " << section;
      logs::db << logs::endl;
    };
  
  doIncrement();
}



bool UserProfile::DecrCredits(const std::string& section, long long kBytes, 
                                bool force, long long& newCredits)
{
  mongo::BSONObjBuilder elemQuery;
  elemQuery.append("section", section);
  if (!force) elemQuery.append("value", BSON("$gte" << kBytes));
  
  auto query = BSON("uid" << profile.ID() << 
                    "credits" << BSON("$elemMatch" << elemQuery.obj()));
                    
  auto update = BSON("$inc" << BSON("credits.$.value" << -kBytes));
                    
  auto cmd = BSON("findandmodify" << "userprofiles" <<
                  "query" << query <<
                  "update" << update);

  db::NoErrorConnection conn;                  
  mongo::BSONObj result;
  bool ret = conn.RunCommand(cmd, result);
  
  return force || (ret && result["value"].type() != mongo::jstNULL);
}

mongo::BSONObj Serialize(const acl::UserProfile& profile)
{
  mongo::BSONObjBuilder bob;
  
  bob.append("modified", ToDateT(profile.modified));
  bob.append("uid", profile.id);
  bob.append("name", profile.name);
  bob.append("ip masks", Serialize(profile.ipMasks));
  bob.append("password", profile.password);
  bob.append("salt", profile.salt);
  bob.append("flags", profile.flags);
  bob.append("primary gid", profile.primaryGid);
  bob.append("secondary gids", Serialize(profile.secondaryGids));
  bob.append("gadmin gids", Serialize(profile.gadminGids));
  bob.append("creator", profile.creator);
  bob.append("created", ToDateT(profile.created));
  bob.append("weekly allotment", profile.weeklyAllotment);
  bob.append("home dir", profile.homeDir);
  bob.append("idle time", profile.idleTime);
  
  if (profile.expires)
    bob.append("expires", ToDateT(*profile.expires));
  else
    bob.appendNull("expires");
    
  bob.append("num logins", profile.numLogins);
  bob.append("comment", profile.comment);
  bob.append("tagline", profile.tagline);
  bob.append("max down speed", profile.maxDownSpeed);
  bob.append("max up speed", profile.maxUpSpeed);
  bob.append("max sim down", profile.maxSimDown);
  bob.append("max sim up", profile.maxSimUp);
  bob.append("logged in", profile.loggedIn);
  
  if (profile.lastLogin)
    bob.append("last login", ToDateT(*profile.lastLogin));
  else
    bob.appendNull("last login");
    
  bob.append("ratio", Serialize(profile.ratio));
  bob.append("credits", Serialize(profile.credits));
  
  return bob.obj();
}

acl::UserProfile Unserialize(const mongo::BSONObj& obj)
{
  try
  { 
    acl::UserProfile profile;
    profile.modified = ToPosixTime(obj["modified"].Date());
    profile.id = obj["uid"].Int();
    profile.name = obj["name"].String();
    profile.ipMasks = Unserialize<decltype(profile.ipMasks)>(obj["ip masks"].Array());
    profile.password = obj["password"].String();
    profile.salt = obj["salt"].String();
    profile.flags = obj["flags"].String();
    profile.primaryGid = obj["primary gid"].Int();
    profile.secondaryGids = Unserialize<decltype(profile.secondaryGids)>(obj["secondary gids"].Array());
    profile.gadminGids = Unserialize<decltype(profile.gadminGids)>(obj["gadmin gids"].Array());
    profile.creator = obj["creator"].Int();

    mongo::BSONElement oid;
    obj.getObjectID(oid);
    profile.created = ToGregDate(oid.OID().asDateT());
    
    profile.weeklyAllotment = obj["weekly allotment"].Long();
    profile.homeDir = obj["home dir"].String();
    profile.idleTime = obj["idle time"].Int();
    
    if (obj["expires"].type() != mongo::jstNULL)
      profile.expires.reset(ToGregDate(obj["expires"].Date()));
    
    profile.numLogins = obj["num logins"].Int();
    profile.comment = obj["comment"].String();
    profile.tagline = obj["tagline"].String();
    profile.maxDownSpeed = obj["max down speed"].Long();
    profile.maxUpSpeed = obj["max up speed"].Long();
    profile.maxSimDown = obj["max sim down"].Int();
    profile.maxSimUp = obj["max sim up"].Int();
    profile.loggedIn = obj["logged in"].Int();

    if (obj["last login"].type() != mongo::jstNULL)
      profile.lastLogin.reset(ToPosixTime(obj["last login"].Date()));
    
    profile.ratio = Unserialize<decltype(profile.ratio)>(obj["ratio"].Array());
    profile.credits = Unserialize<decltype(profile.credits)>(obj["credits"].Array());
    
    return profile;
  }
  catch (const mongo::DBException& e)
  {
    LogException("Unserialize user profile", e, obj);
    throw e;
  }
}

boost::optional<acl::UserProfile> UserProfile::Load(acl::UserID uid)
{
  db::NoErrorConnection conn;                  
  return conn.QueryOne<acl::UserProfile>("users", QUERY("uid" << uid));
}

} /* db namespace */
