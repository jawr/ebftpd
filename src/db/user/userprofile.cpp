#include <sstream>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "db/user/userprofile.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/error.hpp"
#include "db/bson/bson.hpp"
#include "cfg/get.hpp"
#include "ftp/task/task.hpp"
#include "logs/logs.hpp"
#include "util/error.hpp"
#include "acl/userprofile.hpp"
#include "db/bson/userprofile.hpp"

namespace db
{

typedef std::shared_ptr<Task> TaskPtr;
typedef std::vector<mongo::BSONObj> QueryResults;

namespace userprofile
{

void Delete(acl::UserID uid)
{
  Pool::Queue(std::make_shared<db::Delete>("userprofiles", QUERY("uid" << uid)));
}

acl::UserProfile Get(acl::UserID uid)
{
  QueryResults results;
  mongo::Query query = QUERY("uid" << uid);
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  if (results.size() == 0) 
    throw db::DBError("Unable to load user profile.");

  return bson::UserProfile::Unserialize(*results.begin());
}

std::unordered_map<acl::UserID, acl::UserProfile> GetSelection(
  const std::vector<acl::User>& users)
{
  std::unordered_map<acl::UserID, acl::UserProfile> profiles;

  QueryResults results;
  mongo::BSONArrayBuilder b;
  for (auto& user: users)
    b.append(user.UID());
  boost::unique_future<bool> future;
  mongo::Query query = BSON("uid" << BSON("$in" << b.arr()));

  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& result: results)
  {
    acl::UserProfile profile = db::bson::UserProfile::Unserialize(result);
    profiles.insert(std::make_pair(profile.UID(), profile));
  }

  return profiles;
}

void Save(const acl::UserProfile& profile)
{
  mongo::BSONObj obj = db::bson::UserProfile::Serialize(profile);
  mongo::Query query = QUERY("uid" << profile.UID());
  TaskPtr task(new db::Update("userprofiles", query, obj, true));
  Pool::Queue(task);
  std::make_shared<ftp::task::UserUpdate>(profile.UID())->Push();
}

std::vector<acl::UserProfile> GetAll()
{
  std::vector<acl::UserProfile> profiles;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("userprofiles", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    profiles.emplace_back(bson::UserProfile::Unserialize(obj));

  return profiles;
}

void Set(acl::UserID uid, mongo::BSONObj obj)
{
  obj = BSON("$set" << obj);
  mongo::Query query = QUERY("uid" << uid);
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
  std::make_shared<ftp::task::UserUpdate>(uid)->Push();
}

void Unset(acl::UserID uid, const std::string& field)
{
  mongo::BSONObj obj = BSON("$unset" << BSON(field << 1));
  mongo::Query query = QUERY("uid" << uid);
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
  std::make_shared<ftp::task::UserUpdate>(uid)->Push();
}

void SetWeeklyAllotment(acl::UserID uid, int allotment)
{
  Set(uid, BSON("weekly allotment" << allotment));
}
  
void SetHomeDir(acl::UserID uid, const std::string& path)
{
  Set(uid, BSON("home dir" << path));
}

void SetIdleTime(acl::UserID uid, int idleTime)
{
  Set(uid, BSON("idle time" << idleTime));
}

void SetExpires(acl::UserID uid, const boost::optional<boost::gregorian::date>& date)
{
  if (!date) Unset(uid, "expires");
  else Set(uid, BSON("expires" << db::bson::ToDateT(*date)));
}

void SetNumLogins(acl::UserID uid, int logins)
{
  Set(uid, BSON("num logins" << logins));
}

void SetComment(acl::UserID uid, const std::string& comment)
{
  Set(uid, BSON("comment" << comment));
}

void SetMaxDownSpeed(acl::UserID uid, int speed)
{
  Set(uid, BSON("max dl speed" << speed));
}

void SetMaxUpSpeed(acl::UserID uid, int speed)
{
  Set(uid, BSON("max ul speed" << speed));
}

void SetMaxSimDown(acl::UserID uid, int logins)
{
  Set(uid, BSON("max sim dl" << logins));
}

void SetMaxSimUp(acl::UserID uid, int logins)
{
  Set(uid, BSON("max sim ul" << logins));
}

void Login(acl::UserID uid)
{
  // updates login count and time
  mongo::Query query = QUERY("uid" << uid);
  mongo::BSONObj obj = BSON("$inc" << BSON("logged in" << 1) <<
    "$set" << BSON("last login" << mongo::DATENOW));
  TaskPtr task(new db::Update("userprofiles", query, obj, false));
  Pool::Queue(task);
}

void SetRatio(acl::UserID uid, const std::string& section, int ratio)
{
  auto query = QUERY("uid" << uid << "ratio.section" << section);
  auto obj = BSON("$set" << BSON("ratio.$.value" << ratio));
  boost::unique_future<int> future;
  Pool::Queue(std::make_shared<db::Update>("userprofiles", query, obj, future));
  
  if (future.get() < 1)
  {
    query = QUERY("uid" << uid);
    obj = BSON("$push" << 
            BSON("ratio" << 
              BSON("section" << section << "value" << ratio
          )));

    Pool::Queue(std::make_shared<db::Update>("userprofiles", query, obj, future, true));
    future.wait();
  }
  
  std::make_shared<ftp::task::UserUpdate>(uid)->Push();
}

void SetTagline(acl::UserID uid, const std::string& tagline)
{
  Set(uid, BSON("tagline" << tagline));
}

bool DecrCredits(acl::UserID uid, long long kBytes, 
        const std::string& section, bool negativeOkay)
{
  mongo::BSONObjBuilder elemQuery;
  elemQuery.append("section", section);
  if (!negativeOkay) elemQuery.append("value", BSON("$gte" << kBytes));
  
  auto query = BSON("uid" << uid << 
                    "credits" << BSON("$elemMatch" << elemQuery.obj()));
                    
  auto update = BSON("$inc" << BSON("credits.$.value" << -kBytes));
                    
  auto cmd = BSON("findandmodify" << "userprofiles" <<
                  "query" << query <<
                  "update" << update);

  boost::unique_future<bool> future;
  mongo::BSONObj result;
  Pool::Queue(std::make_shared<db::RunCommand>(cmd, result, future));
  future.wait();
  
  return negativeOkay || (future.get() && result["value"].type() != mongo::jstNULL);
}

void IncrCredits(acl::UserID uid, long long kBytes,
        const std::string& section)
{
  auto function = [uid, kBytes, section](mongo::DBClientConnection& conn)
    {
      auto updateExisting = [&]() -> bool
        {
          auto query = BSON("uid" << uid << 
                            "credits" << BSON("$elemMatch" << BSON("section" << section)));
                            
          auto update = BSON("$inc" << BSON("credits.$.value" << kBytes));
                            
          auto cmd = BSON("findandmodify" << "userprofiles" <<
                          "query" << query <<
                          "update" << update);
                          
          mongo::BSONObj result;
          return RunCommand::Execute(conn, cmd, result) && 
                 result["value"].type() != mongo::jstNULL;
        };

      auto doInsert = [&]() -> bool
      {
        auto query = QUERY("uid" << uid << "credits" << BSON("$not" << 
                           BSON("$elemMatch" << BSON("section" << section))));
        auto update = BSON("$push" << BSON("credits" << BSON("section" << section << "value" << kBytes)));
        return Update::Execute(conn, "userprofiles", query, update, false) > 0;
      };
      
      if (updateExisting()) return;
      if (doInsert()) return;
      if (updateExisting()) return;

      logs::db << "Unable to increment credits for UID " << uid;
      if (!section.empty()) logs::db << " in section " << section;
      logs::db << logs::endl;
    };
    
  Pool::Queue(std::make_shared<db::Function>(function));
}

}
}
