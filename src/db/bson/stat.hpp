#ifndef __DB_BSON_STAT_HPP
#define __DB_BSON_STAT_HPP

#include <mongo/client/dbclient.h>
#include "stats/stat.hpp"

namespace db { namespace bson
{

struct Stat
{
  // serialize is probably never needed. adding for consistency.
  static mongo::BSONObj Serialize(const ::stats::Stat& stat);
  static ::stats::Stat Unserialize(const mongo::BSONObj& bo);
  static ::stats::Stat UnserializeRaw(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
