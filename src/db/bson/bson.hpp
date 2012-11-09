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

mongo::Date_t ToDateT(const boost::posix_time::ptime& t);
mongo::Date_t ToDateT(const boost::gregorian::date& d);
boost::posix_time::ptime ToPosixTime(const mongo::Date_t& dt);
boost::gregorian::date ToGregDate(const mongo::Date_t& dt);

} /* bson namespace */
} /* db namespace */

#endif
