#ifndef __DB_GROUP_SERIALIZATION_HPP
#define __DB_GROUP_SERIALIZATION_HPP

#include <mongo/client/dbclient.h>
#include <string>
#include "acl/types.hpp"

namespace db
{

struct GroupPair
{
  std::string name;
  acl::GroupID gid;
};

template <typename T> T Unserialize(const mongo::BSONObj& obj);
template <> inline GroupPair Unserialize<GroupPair>(const mongo::BSONObj& obj)
{
  GroupPair data;
  data.name = obj["name"].String();
  data.gid = obj["gid"].Int();
  return data;
}

} /* db namespace */

#endif
