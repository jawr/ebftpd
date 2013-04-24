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

#ifndef __UTIL_TIME_HPP
#define __UTIL_TIME_HPP

#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace util
{

class Time
{
  boost::posix_time::ptime t;

public:
  Time(time_t t) : t(boost::posix_time::from_time_t(t)) { }
  Time() : t(boost::posix_time::second_clock::local_time()) { }

  int Day() { return t.date().day(); };
  int Week() { return t.date().week_number(); };
  int Month() { return t.date().month(); };
  int Year() { return t.date().year(); };
};

// end
}
#endif
  

