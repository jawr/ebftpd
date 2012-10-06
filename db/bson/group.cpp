#include "db/bson/group.hpp"
#include "acl/group.hpp"

namespace db { namespace bson
{

mongo::BSONObj Group::Serialize(const acl::Group& group)
{
  mongo::BSONObjBuilder bob;
  bob.append("name", group.name);
  bob.append("gid", group.gid);
  return bob.obj();
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
