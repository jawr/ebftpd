#ifndef __DB_STATS_DATE_HPP
#define __DB_STATS_DATE_HPP

#include <boost/date_time/gregorian/gregorian.hpp>

namespace stats
{

class Date
{
  int day, week, month, year;
  
public:
  Date(bool mondayWeekStart)
  {
    boost::gregorian::date today = boost::gregorian::day_clock::local_day();
    day = today.day();
    if (mondayWeekStart) week = today.week_number();
    else week = (today + boost::gregorian::date_duration(1)).week_number();
    month = today.month();
    year = today.year();
  }
  
  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
};

} /* stats namespace */

#endif
