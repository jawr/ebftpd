#ifndef __DB_BSON_STAT_HPP
#define __DB_BSON_STAT_HPP

#include <mongo/client/dbclient.h>
#include "stats/stat.hpp"

namespace acl
{
class Stat;
}

namespace db { namespace bson
{

struct Stat
{
  static mongo::BSONObj Serialize(const stats::Stat& user);
  static stats::Stat Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif

