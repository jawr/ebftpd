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

#ifndef __DB_STATS_DATE_HPP
#define __DB_STATS_DATE_HPP

#include <boost/date_time/gregorian/gregorian.hpp>

namespace stats
{

class Date
{
  int day, week, month, year;
  
public:
  Date()
  {
    boost::gregorian::date today = boost::gregorian::day_clock::local_day();
    day = today.day();
    week = (today + boost::gregorian::date_duration(1)).week_number();
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
