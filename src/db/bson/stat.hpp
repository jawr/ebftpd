#ifndef __DB_BSON_STAT_HPP
#define __DB_BSON_STAT_HPP

namespace mongo
{
class BSONObj;
}

namespace stats
{
class Stat;
}

namespace db { namespace bson
{

struct Stat
{
  static ::stats::Stat Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
