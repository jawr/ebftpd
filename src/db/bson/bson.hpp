#ifndef __DB_BSON_BSON_HPP
#define __DB_BSON_BSON_HPP

#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <mongo/client/dbclient.h>
#include "util/verify.hpp"

namespace db { namespace bson
{

template <typename Container>
mongo::BSONArray SerializeContainer(const Container& c)
{
  mongo::BSONArrayBuilder bab;
  for (const auto& elem : c) bab.append(elem);
  return bab.arr();
}

inline mongo::Date_t ToDateT(const boost::posix_time::ptime& t)
{
  struct tm tm(boost::posix_time::to_tm(t));
  return mongo::Date_t(mktime(&tm));
}

inline mongo::Date_t ToDateT(const boost::gregorian::date& d)
{
  struct tm tm(boost::gregorian::to_tm(d));
  return mongo::Date_t(mktime(&tm));
}

inline boost::posix_time::ptime ToPosixTime(const mongo::Date_t& dt)
{
  return boost::posix_time::from_time_t(dt.toTimeT());
}

inline boost::gregorian::date ToGregDate(const mongo::Date_t& dt)
{
  time_t t = dt.toTimeT();
  struct tm tm;
  verify(!localtime_r(&t, &tm)); // what to do ?!? :(
  return boost::gregorian::date_from_tm(tm);
}

} /* bson namespace */
} /* db namespace */

#endif
