//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/date_time/c_local_time_adjustor.hpp>
#include "db/serialization.hpp"

namespace db
{

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
  return mongo::Date_t(timegm(&tm) * 1000ULL);
}

ptime ToPosixTime(const mongo::Date_t& dt)
{
  mongo::Date_t temp(dt);
  struct tm tm;
  temp.toTm(&tm);
  return local_adj::utc_to_local(from_time_t(timegm(&tm)));
}

} /* db namespace */
