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

#include <cctype>
#include "cmd/util.hpp"
#include "util/string.hpp"

namespace cmd
{

bool SplitArgs(const std::string& command, std::vector<std::string>& args)
{
  args.clear();
  std::string::size_type pos1 = 0;
  std::string::size_type len = command.length();
  while (true)
  {
    while (std::isspace(command[pos1])) ++pos1;
    if (pos1 == len) break;
    
    std::string::size_type pos2 = command[pos1] == '{' ?
                                  command.find_first_of('}', pos1 + 1) :
                                  command.find_first_of(' ', pos1 + 1);
    if (pos2 == std::string::npos)
    {
      if (command[pos1] == '{') return false;
      args.emplace_back(command.substr(pos1));
      util::Trim(args.back());
      break;
    }
    else
    {
      if (command[pos1] == '{') ++pos1;
      args.emplace_back(command.substr(pos1, pos2 - pos1));
      util::Trim(args.back());
      pos1 = pos2 + 1;
    }
  }
  return true;
}

std::string Age(boost::posix_time::time_duration age)
{
  namespace pt = boost::posix_time;
  
  int days = age.hours() / 24;
  age -= pt::hours(days * 24);
  
  int fields = 0;
  if (days > 99) return boost::lexical_cast<std::string>(days) + "d";
  
  std::ostringstream os;
  if (days > 0)
  {
    os << std::setw(2) << days << "d ";
    ++fields;
  }
  
  if (age.hours() > 0)
  {
    os << std::setw(2) << age.hours() << "h ";
    if (++fields >= 2) return os.str();
  }
  
  if (age.minutes() > 0)
  {
    os << std::setw(2) << age.minutes() << "m ";
    if (++fields >= 2) return os.str();
  }
  
  os << std::setw(2) << age.seconds() << "s ";
  return util::TrimCopy(os.str());
}


} /* cmd namespace */
