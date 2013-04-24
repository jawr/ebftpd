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

#ifndef __UTIL_ENUMIOS_HPP
#define __UTIL_ENUMISO_HPP

#include <ostream>
#include <type_traits>
#include "util/enumutil.hpp"

template <
  typename Enum, 
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
std::ostream& operator<<(std::ostream& os, Enum e)
{
  return (os << util::AsUnderlying(e));
}

#endif
