#ifndef __DB_USER_SERIALIZATION_HPP
#define __DB_USER_SERIALIZATION_HPP

#include <mongo/client/dbclient.h>
#include <string>
#include "acl/types.hpp"

namespace db
{

struct UserTriple
{
  std::string name;
  acl::UserID uid;
  acl::GroupID primaryGid;
};

template <typename T> T Unserialize(const mongo::BSONObj& obj);

template <> inline UserTriple Unserialize<UserTriple>(const mongo::BSONObj& obj)
{
  UserTriple data;
  data.name = obj["name"].String();
  data.uid = obj["uid"].Int();
  data.primaryGid = obj["primary gid"].Int();
  return data;
}

} /* db namespace */

#endif
