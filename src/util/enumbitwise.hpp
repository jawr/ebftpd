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
