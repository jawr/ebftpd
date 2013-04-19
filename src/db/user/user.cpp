#include <future>
#include "db/user/user.hpp"
#include "db/connection.hpp"
#include "acl/user.hpp"
#include "db/serialization.hpp"
#include "db/error.hpp"
#include "db/user/util.hpp"
#include "db/group/util.hpp"
#include "util/futureminder.hpp"
#include "acl/userdata.hpp"

namespace db
{

bool User::Create()
{
  NoErrorConnection conn;
  user.id = conn.InsertAutoIncrement("users", user, "uid");
  if (user.id == -1) return false;
  UpdateLog();
  return true;
}

void User::UpdateLog() const
{
  FastConnection conn;
  auto entry = BSON("collection" << "users" << "id" << user.id);
  conn.Insert("updatelog", entry);
}

void User::SaveField(const std::string& field, bool updateLog) const
{
  NoErrorConnection conn;
  conn.SetField("users", QUERY("uid" << user.id), user, field);
  if (updateLog) UpdateLog();
}

bool User::SaveName()
{
  try
  {
    SafeConnection conn;
    conn.SetField("users", QUERY("uid" << user.id), user, "name" );
    UpdateLog();
    return true;
  }
  catch (const DBError&)
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
  NoErrorConnection conn;
  conn.SetFields("users", QUERY("uid" << user.id), user, { "password", "salt" });
  UpdateLog();
}

void User::SaveFlags()
{
  SaveField("flags");
}

void User::SaveGIDs()
{
  NoErrorConnection conn;
  conn.SetFields("users", QUERY("uid" << user.id), user, { "primary gid", "secondary gids", "gadmin gids" });
  UpdateLog();
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
  NoErrorConnection conn;
  conn.SetFields("users", QUERY("uid" << user.id), user, { "logged in", "last login" });
}

void User::SaveRatio()
{
  SaveField("ratio");
}

namespace
{
util::FutureMinder asyncTasks;
}

void User::IncrCredits(const std::string& section, long long kBytes)
{
  auto doIncrement = [section, kBytes](acl::UserID uid)
    {
      NoErrorConnection conn;
      auto updateExisting = [&]() -> bool
        {
          auto query = BSON("uid" << uid << 
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
        auto query = QUERY("uid" << uid << "credits" << BSON("$not" << 
                           BSON("$elemMatch" << BSON("section" << section))));
        auto update = BSON("$push" << BSON("credits" << BSON("section" << section << "value" << kBytes)));
        return conn.Update("users", query, update, false) > 0;
      };
      
      if (updateExisting()) return;
      if (doInsert()) return;
      if (updateExisting()) return;

      logs::Database("Unable to increment credits for UID %1%%2%", uid,
                     !section.empty() ? " in section " + section : 
                     std::string(""));
    };
  
  asyncTasks.Assign(std::async(std::launch::async, doIncrement, user.id));
}

bool User::DecrCredits(const std::string& section, long long kBytes, bool force)
{
  if (!kBytes) return true;
  
  mongo::BSONObjBuilder elemQuery;
  elemQuery.append("section", section);
  if (!force) elemQuery.append("value", BSON("$gte" << kBytes));
  
  auto query = BSON("uid" << user.id << 
                    "credits" << BSON("$elemMatch" << elemQuery.obj()));
                    
  auto update = BSON("$inc" << BSON("credits.$.value" << -kBytes));
                    
  auto cmd = BSON("findandmodify" << "users" <<
                  "query" << query <<
                  "update" << update);
  NoErrorConnection conn;                  
  mongo::BSONObj result;
  bool ret = conn.RunCommand(cmd, result);
  return force || (ret && result["value"].type() != mongo::jstNULL);
}

void User::Purge() const
{
  NoErrorConnection conn;
  conn.Remove("users", QUERY("uid" << user.id));
  UpdateLog();
}

template <> mongo::BSONObj Serialize<acl::UserData>(const acl::UserData& user)
{
  mongo::BSONObjBuilder bob;

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
    
  bob.append("ratio", SerializeMap(user.ratio, "section", "value"));
  bob.append("credits", SerializeMap(user.credits, "section", "value"));
  bob.append("weekly allotment", SerializeMap(user.weeklyAllotment, "section", "value"));
  
  return bob.obj();
}

template <> acl::UserData Unserialize<acl::UserData>(const mongo::BSONObj& obj)
{
  try
  { 
    acl::UserData user;
    user.id = obj["uid"].Int();
    user.name = obj["name"].String();
    UnserializeContainer(obj["ip masks"].Array(), user.ipMasks);
    user.password = obj["password"].String();
    user.salt = obj["salt"].String();
    user.flags = obj["flags"].String();
    user.primaryGid = obj["primary gid"].Int();
    UnserializeContainer(obj["secondary gids"].Array(), user.secondaryGids);
    UnserializeContainer(obj["gadmin gids"].Array(), user.gadminGids);
    
    user.creator = obj["creator"].Int();
    mongo::BSONElement oid;
    obj.getObjectID(oid);
    user.created = ToGregDate(oid.OID().asDateT());
    
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
    
    UnserializeMap(obj["ratio"].Array(), "section", "value", user.ratio);
    UnserializeMap(obj["credits"].Array(), "section", "value", user.credits);
    UnserializeMap(obj["weekly allotment"].Array(), "section", "value", user.weeklyAllotment);
    
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
  NoErrorConnection conn;                  
  return conn.QueryOne<acl::UserData>("users", QUERY("uid" << uid));
}

boost::optional<acl::UserData> User::Load(const std::string& name)
{
  NoErrorConnection conn;                  
  return conn.QueryOne<acl::UserData>("users", QUERY("name" << name));
}

namespace
{

template <typename T>
std::vector<T> GetUsersGeneric(const std::string& multiStr, const mongo::BSONObj* fields)
{
  std::vector<std::string> toks;
  util::Split(toks, multiStr, " ", true);
  
  mongo::Query query;
  if (std::find(toks.begin(), toks.end(), "*") == toks.end())
  {
    mongo::BSONArrayBuilder namesBab;
    mongo::BSONArrayBuilder gidsBab;
    
    for (std::string tok : toks)
    {
      if (tok[0] == '=')
      {
        acl::GroupID gid = NameToGID(tok.substr(1));
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

  NoErrorConnection conn;
  return conn.QueryMulti<T>("users", query, 0, 0, fields);
}

}

std::vector<acl::UserID> GetUIDs(const std::string& multiStr)
{
  auto fields = BSON("uid" << 1);
  return GetUsersGeneric<acl::UserID>(multiStr, &fields);
}

std::vector<acl::UserData> GetUsers(const std::string& multiStr)
{
  return GetUsersGeneric<acl::UserData>(multiStr, nullptr);
}

} /* db namespace */
