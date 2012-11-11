#ifndef __DB_STATS_DATE_HPP
#define __DB_STATS_DATE_HPP

#include <boost/date_time/gregorian/gregorian.hpp>

namespace db { namespace stats
{

class Date
{
  boost::gregorian::date date;
  
public:
  Date() : date(boost::gregorian::day_clock::local_day()) { }
  
  int Day() const { return date.day(); }
  int Week() const { return date.week_number(); }
  int Month() const { return date.month(); }
  int Year() const { return date.year(); }
};

} /* stats namespace */
} /* db namespace */

#endif
