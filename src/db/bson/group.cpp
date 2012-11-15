#include "db/bson/group.hpp"
#include "acl/group.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

mongo::BSONObj Group::Serialize(const acl::Group& group)
{
  mongo::BSONObjBuilder bob;
  bob.append("name", group.name);
  bob.append("gid", group.gid);
  return bob.obj();
}

std::unique_ptr<acl::Group> Group::Unserialize(const mongo::BSONObj& bo)
{
  std::unique_ptr<acl::Group> group;
  try
  {
    group.reset(new acl::Group(bo["name"].String(), bo["gid"].Int()));
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("group", e, bo);
  }
  return group;
}

acl::Group Group::Unserialize(const mongo::BSONObj& bo)
{
  return acl::Group(bo["name"].String(), bo["gid"].Int());
}

} /* bson namespace */
} /* db namespace */

#ifdef DB_BSON_GROUP_TEST

#include <iostream>

int main()
{
  using namespace db::bson;
  using namespace mongo;
  
  acl::Group group("somegroup", 123);
  
  BSONObj bo = Group::Serialize(group);
  std::cout << bo["name"].String() << " " << bo["gid"].Int() << std::endl;
  group = Group::Unserialize(bo);
  std::cout << group.Name() << " " << group.GID() << std::endl;
}

#endif
