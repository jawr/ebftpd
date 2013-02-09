#ifndef __UTIL_ENUMUTIL_HPP
#define __UTIL_ENUMUTIL_HPP

#if defined(__GNUC__)
#include <features.h>
#if __GNUC_PREREQ(4,7)
#include <type_traits>
#else

#include <tuple>

// This is a hack because GCC 4.6 does not support std::underlying_type yet.
// A specialization for each enum is preferred
namespace detail {
  template <typename T, typename Acc, typename... In>
  struct filter;

  template <typename T, typename Acc>
  struct filter<T, Acc> {
    typedef typename std::tuple_element<0, Acc>::type type;
  };

  template <typename T, typename... Acc, typename Head, typename... Tail>
  struct filter<T, std::tuple<Acc...>, Head, Tail...>
  : std::conditional<sizeof(T) == sizeof(Head) && (T(-1) < T(0)) == (Head(-1) < Head(0))
            , filter<T, std::tuple<Acc...,Head>, Tail...>
            , filter<T, std::tuple<Acc...>, Tail...>
            >::type {};

  template <typename T, typename... In>
  struct find_best_match : filter<T, std::tuple<>, In...> {};
}

namespace std {
  template <typename E>
  struct underlying_type : detail::find_best_match<E,
                signed short,
                unsigned short,
                signed int,
                unsigned int,
                signed long,
                unsigned long,
                signed long long,
                unsigned long long,
                bool,
                char,
                signed char,
                unsigned char,
                wchar_t,
                char16_t,
                char32_t> {};
}

#endif
#endif

namespace util
{

template <
  typename Enum,
  typename Underlying = typename std::underlying_type<Enum>::type
>
Underlying AsUnderlying(Enum e)
{
  return static_cast<Underlying>(e);
}

} /* util namespace */

#endif
