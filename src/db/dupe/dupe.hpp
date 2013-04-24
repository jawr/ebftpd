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

#ifndef __DB_DUPE_DUPE_HPP
#define __DB_DUPE_DUPE_HPP

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace db { namespace dupe
{

void Add(const std::string& directory, const std::string& section);

struct DupeResult
{
  std::string directory;
  std::string section;
  boost::posix_time::ptime dateTime;
  
  DupeResult(const std::string& directory, const std::string& section, 
               const boost::posix_time::ptime& dateTime) :
    directory(directory), section(section), dateTime(dateTime)
  { }
};

std::vector<DupeResult> Search(const std::vector<std::string>& terms, int limit);

} /* dupe namespace */
} /* db namespace */

#endif
