#include <sstream>
#include "db/acl/acl.hpp"
#include "db/bson/user.hpp"
#include "acl/groupcache.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"

namespace db
{

util::Error GetUsersByACL(boost::ptr_vector<acl::User>& users,
  std::string& acl)
{
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
      return util::Error::Failure(e.Message());
    }
    query = QUERY("$or" << BSON_ARRAY(BSON("primary gids" << group.GID()) << BSON("secondary gids" << group.GID())));
  }
  else
    query = QUERY("flags" << acl);

  QueryResults results;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("users", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    users.push_back(bson::User::Unserialize(obj));

  return util::Error::Success();
}

// end
}
