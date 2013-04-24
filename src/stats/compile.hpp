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

#ifndef __STATS_COMPILE_HPP
#define __STATS_COMPILE_HPP

#include <string>
#include <functional>
#include "stats/types.hpp"

namespace text
{
class Template;
}

namespace acl
{
class User;
}

namespace stats
{

std::string CompileUserRanks(const std::string& section, Timeframe tf, Direction dir, SortField sf, int max, 
                             text::Template& templ, const std::function<bool(acl::User&)>& filter = nullptr);
std::string CompileGroupRanks(const std::string& section, Timeframe tf, Direction dir, SortField sf, int max, 
                              text::Template& templ);

} /* stats namespace */

#endif
