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

#ifndef __DB_INDEX_INDEX_HPP
#define __DB_INDEX_INDEX_HPP

#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace db { namespace index
{

void Add(const std::string& path, const std::string& section);
void Delete(const std::string& path);

struct SearchResult
{
  std::string path;
  std::string section;
  boost::posix_time::ptime dateTime;
  
  SearchResult(const std::string& path, const std::string& section, 
               const boost::posix_time::ptime& dateTime) :
    path(path), section(section), dateTime(dateTime)
  { }
};

std::vector<SearchResult> Search(const std::vector<std::string>& terms, int limit);
std::vector<SearchResult> Newest(int limit);

} /* index namespace */
} /* db namespace */

#endif
