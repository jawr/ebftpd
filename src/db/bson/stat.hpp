#ifndef __DB_BSON_STAT_HPP
#define __DB_BSON_STAT_HPP

#include <mongo/client/dbclient.h>
#include "stats/stat.hpp"

namespace db { namespace bson
{

struct Stat
{
  static ::stats::Stat Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
