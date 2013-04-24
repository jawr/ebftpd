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

#include "cfg/util.hpp"
#include "util/string.hpp"

namespace cfg
{

bool YesNoToBoolean(std::string s)
{
  util::ToLower(s);
  if (s == "yes") return true;
  if (s == "no") return false;
  throw std::bad_cast();
}

long long ParseSize(std::string s)
{
  if (s.empty()) throw std::bad_cast();
  
  std::string unit = "K";
  if (std::isalpha(s.back()))
  {
    unit.assign(s.end() - 1, s.end());
    s.erase(s.end() - 1, s.end());
    if (s.empty()) throw std::bad_cast();
    util::ToUpper(unit);
  }
  
  long long kBytes = util::StrToLLong(s);
  if (kBytes < 0) throw std::bad_cast();
  
  if (unit == "M") kBytes *= 1024;
  else if (unit == "G") kBytes *= 1024 * 1024;
  else if (unit != "K") throw std::bad_cast();
  
  return kBytes;
}

} /* cfg namespace */
