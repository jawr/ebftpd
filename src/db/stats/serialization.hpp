#ifndef __DB_STATS_SERIALIZATION_HPP
#define __DB_STATS_SERIALIZATION_HPP

namespace mongo
{
class BSONObj;
}

namespace stats
{
enum class Timeframe : unsigned;
class Stat;
}

namespace db { namespace stats
{

mongo::BSONObj Serialize(::stats::Timeframe timeframe);
::stats::Stat Unserialize(const mongo::BSONObj& obj);

} /* stats namespace */
} /* db namespace */

#endif
