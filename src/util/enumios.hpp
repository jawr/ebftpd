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
