#include <boost/date_time/c_local_time_adjustor.hpp>
#include "db/bson/bson.hpp"

namespace db { namespace bson {

using namespace boost::posix_time;
using namespace boost::gregorian;

typedef boost::date_time::c_local_adjustor<ptime> local_adj;

time_duration UTCOffset()
{
  ptime utcNow = second_clock::universal_time();
  return local_adj::utc_to_local(utcNow) - utcNow;
}

mongo::Date_t ToDateT(const ptime& t)
{
  struct tm tm(to_tm(t - UTCOffset()));
  return mongo::Date_t(timegm(&tm) * 1000);
}

ptime ToPosixTime(const mongo::Date_t& dt)
{
  struct tm tm;
  const_cast<mongo::Date_t*>(&dt)->toTm(&tm);
  return local_adj::utc_to_local(from_time_t(timegm(&tm)));
}

} /* bson namespace */
} /* db namespace */
