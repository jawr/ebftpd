#ifndef __DB_STATS_TRANSFERS_HPP
#define __DB_STATS_TRANSFERS_HPP

#include <string>
#include "acl/types.hpp"
#include "stats/types.hpp"
#include "db/stats/traffic.hpp"

namespace db { namespace stats
{

Traffic TransfersUser(acl::UserID uid, ::stats::Timeframe timeframe, const std::string& section);
Traffic TransfersTotal(::stats::Timeframe timeframe, const std::string& section);

} /* stats namespace */
} /* db namespace */

#endif
