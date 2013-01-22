#ifndef __DB_BSON_TIMEFRAME_HPP
#define __DB_BSON_TIMEFRAME_HPP

#include <mongo/client/dbclient.h>
#include "stats/types.hpp"

namespace db { namespace bson
{

mongo::BSONObj TimeframeSerialize(::stats::Timeframe timeframem);

} /* bson namespace */
} /* db namespace */

#endif
