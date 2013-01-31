#include <sstream>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "db/user/userprofile.hpp"
#include "db/task.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"
#include "db/bson/userprofile.hpp"
#include "db/error.hpp"
#include "db/bson/bson.hpp"
#include "cfg/get.hpp"
#include "ftp/task/task.hpp"

namespace db { namespace userprofile
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
    profiles.push_back(bson::UserProfile::Unserialize(obj));

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

util::Error SetWeeklyAllotment(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number of 0 or larger.");
  }
  Set(uid, BSON("weekly allotment" << i));
  return util::Error::Success();
}
  
util::Error SetHomeDir(acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("home dir" << value));
  return util::Error::Success();
}

util::Error SetStartupDir(acl::UserID uid, const std::string& value)
{
  Set(uid, BSON("startup dir" << value));
  return util::Error::Success();
}

util::Error SetIdleTime(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < -1) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number of -1 or larger.");
  }
  Set(uid, BSON("idle time" << i));
  return util::Error::Success();
}

util::Error SetExpires(acl::UserID uid, std::string& value)
{
  boost::to_lower(value);
  if (value == "never")
  {
    Unset(uid, "expires");
  }
  else
  {
    try
    {
      Set(uid, BSON("expires" << db::bson::ToDateT(boost::gregorian::from_simple_string(value))));
    }
    catch (const std::exception&)
    {
      return util::Error::Failure("Invalid date. Must be in format YYYY-MM-DD or NEVER.");
    }
  }
  return util::Error::Success();
}

util::Error SetNumLogins(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i <= 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number larger than 0.");
  }
  Set(uid, BSON("num logins" << i));
  return util::Error::Success();
}

util::Error SetComment(acl::UserID uid, const std::string& value)
{
  
  Set(uid, BSON("comment" << value));
  return util::Error::Success();
}

util::Error SetMaxDlSpeed(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number of 0 or larger.");
  }
  Set(uid, BSON("max dl speed" << i));
  return util::Error::Success();
}

util::Error SetMaxUlSpeed(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number of 0 or larger.");
  }
  Set(uid, BSON("max ul speed" << i));
  return util::Error::Success();
}

util::Error SetMaxSimDl(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < -1) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be number of -1 or larger.");
  }
  Set(uid, BSON("max sim dl" << i));
  return util::Error::Success();
}

util::Error SetMaxSimUl(acl::UserID uid, const std::string& value)
{
  int i;
  try
  {
    i = boost::lexical_cast<int>(value);
    if (i < -1) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    return util::Error::Failure("Invalid value. Must be a number of -1 or larger.");
  }
  Set(uid, BSON("max sim ul" << i));
  return util::Error::Success();
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
