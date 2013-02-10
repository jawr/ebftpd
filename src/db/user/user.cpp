#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/user/user.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/user.hpp"
#include "db/bson/userprofile.hpp"
#include "db/bson/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/group.hpp"
#include "db/error.hpp"
#include "db/bson/bson.hpp"

namespace db
{

typedef std::shared_ptr<Task> TaskPtr;
typedef std::vector<mongo::BSONObj> QueryResults;

namespace user
{

bool Create(acl::User& user)
{
  static const char* javascript =
    "function newUser(user) {\n"
    "  var users = db['users'];\n"
    "  while (1) {\n"
    "    var cursor = users.find({}, {uid : 1}).sort({uid : -1}).limit(1);\n"
    "    user.uid = cursor.hasNext() ? cursor.next().uid + 1 : 0;\n"
    "    users.insert(user);\n"
    "    var err = db.getLastErrorObj();\n"
    "    if (err &&  err.code == 11000) {\n"
    "      if (users.findOne({uid : user.uid}))\n"
    "        continue;\n"
    "      else\n"
    "        return -1;\n"
    "    }\n"
    "    break;\n"
    "  }\n"
    "  return user.uid\n"
    "}";
    
  auto args = db::bson::User::Serialize(user);
  mongo::BSONElement ret;
  boost::unique_future<bool> future;
  
  auto task = std::make_shared<db::Eval>(javascript, args, ret, future);
  Pool::Queue(task);
  if (!future.get()) throw DBError("Error while creating user.");

  user.uid = static_cast<acl::UserID>(ret.Double());
  return user.uid != -1;
}

void Save(const acl::User& user)
{
  mongo::BSONObj obj = db::bson::User::Serialize(user);
  mongo::Query query = QUERY("uid" << user.UID());
  TaskPtr task(new db::Update("users", query, obj, true));
  Pool::Queue(task);
}

void Save(const acl::User& user, const std::string& field)
{
  mongo::BSONObj userObj = db::bson::User::Serialize(user);
  mongo::Query query = QUERY("uid" << user.UID());
  mongo::BSONObj obj = BSON("$set" << BSON(field << userObj[field]) <<
                            "$set" << BSON("modified" << db::bson::ToDateT(user.Modified())));
  TaskPtr task(new db::Update("users", query, obj, false));
  Pool::Queue(task);
}

void Delete(acl::UserID uid)
{
  mongo::Query query = QUERY("uid" << uid);
  Pool::Queue(std::make_shared<db::Delete>("users", query));
  Pool::Queue(std::make_shared<db::Delete>("ipmasks", query));
  Pool::Queue(std::make_shared<db::Delete>("transfers", query));
}

boost::ptr_vector<acl::User> 
GetAllPtr(const boost::optional<boost::posix_time
    ::ptime>& modified)
{
  boost::ptr_vector<acl::User> users;

  QueryResults results;
  mongo::Query query;
  if (modified) query = QUERY("modified" << BSON("$gte" << db::bson::ToDateT(*modified)));
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.push_back(bson::User::UnserializePtr(obj).release());

  return users;
}

std::vector<acl::User> GetAll()
{
  std::vector<acl::User> users;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.emplace_back(bson::User::Unserialize(obj));

  return users;
}


// change to objects rather than pointers
std::vector<acl::User> GetByACL(std::string acl)
{
  std::vector<acl::User> users;

  mongo::Query query;
  if (acl[0] == '-')
  {
    acl.assign(acl.begin()+1, acl.end());
    query = QUERY("name" << acl);
  }
  else if (acl[0] == '=')
  {
    acl.assign(acl.begin()+1, acl.end());
    acl::Group group;
    try
    {
      group = acl::GroupCache::Group(acl);
    }
    catch (const util::RuntimeError& e)
    {
      // group not found
      return users;
    }
    query = QUERY("$or" << BSON_ARRAY(
      BSON("primary gid" << group.GID()) << 
      BSON("secondary gids" << group.GID())
    ));
  }
  else
    query = QUERY("flags" << acl);

  QueryResults results;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.emplace_back(bson::User::Unserialize(obj));

  return users;
}

void SaveIPMasks(const acl::User& user)
{
  auto query = QUERY("uid" << user.UID());
  auto update = BSON("$set" << BSON("ip masks" << db::bson::SerializeContainer(user.ListIPMasks())) <<
                     "$set" << BSON("modified" << db::bson::ToDateT(user.Modified())));
  Pool::Queue(std::make_shared<db::Update>("users", query, update, true));
}

std::vector<acl::UserID> GetMultiUIDOnly(const std::string& multiStr)
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
        acl::GroupID gid = acl::GroupCache::NameToGID(tok.substr(1));
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
  
  QueryResults results;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future, 0, 0, BSON("uid" << 1)));
  Pool::Queue(task);

  future.wait();

  std::vector<acl::UserID> uids;
  for (auto& result : results)
  {
    uids.emplace_back(result["uid"].Int());
  }
  
  return uids;
}


// end
}
}

