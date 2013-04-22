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

#ifndef __UTIL_ENUMBITWISE_HPP
#define __UTIL_ENUMBITWISE_HPP

#include <type_traits>
#include "util/enumutil.hpp"

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum operator|(Enum e1, Enum e2)
{
  typedef typename std::underlying_type<Enum>::type enum_type;
  return static_cast<Enum>(util::AsUnderlying(e1) | util::AsUnderlying(e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum& operator|=(Enum& e1, Enum e2)
{
  return (e1 = operator|(e1, e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum operator&(Enum e1, Enum e2)
{
  typedef typename std::underlying_type<Enum>::type enum_type;
  return static_cast<Enum>(util::AsUnderlying(e1) & util::AsUnderlying(e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum& operator&=(Enum& e1, Enum e2)
{
  return (e1 = operator&(e1, e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum operator^(Enum e1, Enum e2)
{
  typedef typename std::underlying_type<Enum>::type enum_type;
  return static_cast<Enum>(util::AsUnderlying(e1) ^ util::AsUnderlying(e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum& operator^=(Enum& e1, Enum e2)
{
  return (e1 = operator^(e1, e2));
}

template <
  typename Enum,
  typename = typename std::enable_if<std::is_enum<Enum>::value>::type
>
Enum operator~(Enum e)
{
  typedef typename std::underlying_type<Enum>::type enum_type;
  return static_cast<Enum>(~util::AsUnderlying(e));
}

#endif
