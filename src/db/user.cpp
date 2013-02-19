#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "db/user.hpp"
#include "db/connection.hpp"
#include "acl/user.hpp"
#include "db/serialization.hpp"
#include "db/error.hpp"
#include "db/group.hpp"
#include "db/usercache.hpp"

namespace db
{

acl::UserID User::Create()
{
  db::SafeConnection conn;
  return conn.InsertAutoIncrement("users", user, "uid");
}

void User::SaveField(const std::string& field)
{
  db::NoErrorConnection conn;
  conn.SetField("users", QUERY("uid" << user.id), user, { field, "modified" });
}

bool User::SaveName()
{
  try
  {
    SafeConnection conn;
    conn.SetField("users", QUERY("uid" << user.id), user, { std::string("name"), "modified" });
    return true;
  }
  catch (const db::DBError&)
  {
    return false;
  }
}

void User::SaveIPMasks()
{
  SaveField("ip masks");
}

void User::SavePassword()
{
  SaveField("password");
}

void User::SaveFlags()
{
  SaveField("flags");
}

void User::SavePrimaryGID()
{
  SaveField("primary gid");
}

void User::SaveSecondaryGIDs()
{
  SaveField("secondary gids");
}

void User::SaveGadminGIDs()
{
  SaveField("gadmin gids");
}

void User::SaveWeeklyAllotment()
{
  SaveField("weekly allotment");
}

void User::SaveHomeDir()
{
  SaveField("home dir");
}

void User::SaveIdleTime()
{
  SaveField("idle time");
}

void User::SaveExpires()
{
  SaveField("expires");
}

void User::SaveNumLogins()
{
  SaveField("num logins");
}

void User::SaveComment()
{
  SaveField("comment");
}

void User::SaveTagline()
{
  SaveField("tagline");
}

void User::SaveMaxDownSpeed()
{
  SaveField("max down speed");
}

void User::SaveMaxUpSpeed()
{
  SaveField("max up speed");
}

void User::SaveMaxSimDown()
{
  SaveField("max sim down");
}

void User::SaveMaxSimUp()
{
  SaveField("max sim up");
}

void User::SaveLoggedIn()
{
  SaveField("logged in");
}

void User::SaveLastLogin()
{
  SaveField("last login");
}

void User::SaveRatio()
{
  SaveField("ratio");
}

void User::IncrCredits(const std::string& section, long long kBytes)
{
  // in the old thread pool design this used be done in a separate thread
  // this may need to be changed to be async again.
  db::FastConnection conn;
  
  auto doIncrement = [&]()
    {
      auto updateExisting = [&]() -> bool
        {
          auto query = BSON("uid" << user.id << 
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
        auto query = QUERY("uid" << user.id << "credits" << BSON("$not" << 
                           BSON("$elemMatch" << BSON("section" << section))));
        auto update = BSON("$push" << BSON("credits" << BSON("section" << section << "value" << kBytes)));
        return conn.Update("users", query, update, false) > 0;
      };
      
      if (updateExisting()) return;
      if (doInsert()) return;
      if (updateExisting()) return;

      logs::db << "Unable to increment credits for UID " << user.id;
      if (!section.empty()) logs::db << " in section " << section;
      logs::db << logs::endl;
    };
  
  doIncrement();
}

void User::Purge() const
{
  NoErrorConnection conn;
  conn.Remove("users", QUERY("uid" << user.id));
}

bool User::DecrCredits(const std::string& section, long long kBytes, bool force)
{
  mongo::BSONObjBuilder elemQuery;
  elemQuery.append("section", section);
  if (!force) elemQuery.append("value", BSON("$gte" << kBytes));
  
  auto query = BSON("uid" << user.id << 
                    "credits" << BSON("$elemMatch" << elemQuery.obj()));
                    
  auto update = BSON("$inc" << BSON("credits.$.value" << -kBytes));
                    
  auto cmd = BSON("findandmodify" << "users" <<
                  "query" << query <<
                  "update" << update);

  db::NoErrorConnection conn;                  
  mongo::BSONObj result;
  bool ret = conn.RunCommand(cmd, result);
  
  return force || (ret && result["value"].type() != mongo::jstNULL);
}

template <> mongo::BSONObj Serialize<acl::UserData>(const acl::UserData& user)
{
  mongo::BSONObjBuilder bob;

  bob.append("modified", ToDateT(user.modified));
  bob.append("uid", user.id);
  bob.append("name", user.name);
  bob.append("ip masks", SerializeContainer(user.ipMasks));
  bob.append("password", user.password);
  bob.append("salt", user.salt);
  bob.append("flags", user.flags);
  bob.append("primary gid", user.primaryGid);
  bob.append("secondary gids", SerializeContainer(user.secondaryGids));
  bob.append("gadmin gids", SerializeContainer(user.gadminGids));
  bob.append("creator", user.creator);
  bob.append("created", ToDateT(user.created));
  bob.append("weekly allotment", user.weeklyAllotment);
  bob.append("home dir", user.homeDir);
  bob.append("idle time", user.idleTime);
  
  if (user.expires)
    bob.append("expires", ToDateT(*user.expires));
  else
    bob.appendNull("expires");
    
  bob.append("num logins", user.numLogins);
  bob.append("comment", user.comment);
  bob.append("tagline", user.tagline);
  bob.append("max down speed", user.maxDownSpeed);
  bob.append("max up speed", user.maxUpSpeed);
  bob.append("max sim down", user.maxSimDown);
  bob.append("max sim up", user.maxSimUp);
  bob.append("logged in", user.loggedIn);
  
  if (user.lastLogin)
    bob.append("last login", ToDateT(*user.lastLogin));
  else
    bob.appendNull("last login");
    
  bob.append("ratio", SerializeContainer(user.ratio));
  bob.append("credits", SerializeContainer(user.credits));
  
  return bob.obj();
}

template <> acl::UserData Unserialize<acl::UserData>(const mongo::BSONObj& obj)
{
  try
  { 
    acl::UserData user;
    user.modified = ToPosixTime(obj["modified"].Date());
    user.id = obj["uid"].Int();
    user.name = obj["name"].String();
    user.ipMasks = UnserializeContainer<decltype(user.ipMasks)>(obj["ip masks"].Array());
    user.password = obj["password"].String();
    user.salt = obj["salt"].String();
    user.flags = obj["flags"].String();
    user.primaryGid = obj["primary gid"].Int();
    user.secondaryGids = UnserializeContainer<decltype(user.secondaryGids)>(obj["secondary gids"].Array());
    user.gadminGids = UnserializeContainer<decltype(user.gadminGids)>(obj["gadmin gids"].Array());
    user.creator = obj["creator"].Int();

    mongo::BSONElement oid;
    obj.getObjectID(oid);
    user.created = ToGregDate(oid.OID().asDateT());
    
    user.weeklyAllotment = obj["weekly allotment"].Long();
    user.homeDir = obj["home dir"].String();
    user.idleTime = obj["idle time"].Int();
    
    if (obj["expires"].type() != mongo::jstNULL)
      user.expires.reset(ToGregDate(obj["expires"].Date()));
    
    user.numLogins = obj["num logins"].Int();
    user.comment = obj["comment"].String();
    user.tagline = obj["tagline"].String();
    user.maxDownSpeed = obj["max down speed"].Long();
    user.maxUpSpeed = obj["max up speed"].Long();
    user.maxSimDown = obj["max sim down"].Int();
    user.maxSimUp = obj["max sim up"].Int();
    user.loggedIn = obj["logged in"].Int();

    if (obj["last login"].type() != mongo::jstNULL)
      user.lastLogin.reset(ToPosixTime(obj["last login"].Date()));
    
    user.ratio = UnserializeContainer<decltype(user.ratio)>(obj["ratio"].Array());
    user.credits = UnserializeContainer<decltype(user.credits)>(obj["credits"].Array());
    
    return user;
  }
  catch (const mongo::DBException& e)
  {
    LogException("Unserialize user", e, obj);
    throw e;
  }
}

boost::optional<acl::UserData> User::Load(acl::UserID uid)
{
  db::NoErrorConnection conn;                  
  return conn.QueryOne<acl::UserData>("users", QUERY("uid" << uid));
}

namespace
{
std::shared_ptr<UserCacheBase> cache(new UserNoCache());
}

std::string UIDToName(acl::UserID uid)
{
  assert(cache);
  return cache->UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  assert(cache);
  return cache->NameToUID(name);
}

acl::GroupID UIDToPrimaryGID(acl::UserID uid)
{
  assert(cache);
  return cache->UIDToPrimaryGID(uid);
}

namespace
{

template <typename T>
std::vector<T> GetGeneric(const std::string& multiStr, const mongo::BSONObj* fields)
{
  std::vector<std::string> toks;
  boost::split(toks, multiStr, boost::is_any_of(" "), boost::token_compress_on);
  
  mongo::Query query;
  if (std::find(toks.begin(), toks.end(), "*") == toks.end())
  {
    mongo::BSONArrayBuilder namesBab;
    mongo::BSONArrayBuilder gidsBab;
    
    for (std::string tok : toks)
    {
      if (tok[0] == '=')
      {
        acl::GroupID gid = db::NameToGID(tok.substr(1));
        if (gid != -1)
        {
          gidsBab.append(gid);
        }
        continue;
      }
      
      if (tok[0] == '-') tok.erase(0, 1);
      namesBab.append(tok);
    }
    
    auto gids = gidsBab.arr();
    query = QUERY("$or" << 
      BSON_ARRAY(BSON("name" << BSON("$in" << namesBab.arr())) <<
                 BSON("primary gid" << BSON("$in" << gids)) <<
                 BSON("secondary gids" << BSON("$in" << gids))));
  }
  
  db::NoErrorConnection conn;
  return conn.QueryMulti<T>("users", query, 0, 0, fields);
}

}

std::vector<acl::UserID> GetUIDs(const std::string& multiStr)
{
  auto fields = BSON("uid" << 1);
  return GetGeneric<acl::UserID>(multiStr, &fields);
}

std::vector<acl::UserData> GetUsers(const std::string& multiStr)
{
  return GetGeneric<acl::UserData>(multiStr, nullptr);
}

} /* db namespace */
