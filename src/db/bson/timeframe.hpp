#ifndef __DB_BSON_TIMEFRAME_HPP
#define __DB_BSON_TIMEFRAME_HPP

namespace mongo
{
class BSONObj;
}

namespace stats
{
enum class Timeframe : unsigned;
}

namespace db { namespace bson
{

mongo::BSONObj TimeframeSerialize(::stats::Timeframe timeframe);

} /* bson namespace */
} /* db namespace */

#endif
