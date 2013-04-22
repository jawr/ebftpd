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

#ifndef __UTIL_TYPETRAITS_HPP
#define __UTIL_TYPETRAITS_HPP

#include <type_traits>
#include <iterator>
#include <boost/type_traits/ice.hpp>

namespace util
{

// stolen from http://stackoverflow.com/questions/11383722/adl-does-not-work-in-the-specific-situation
// and modified

template <typename T>
typename std::add_rvalue_reference<T>::type declval(); // vs2010 does not support std::declval - workaround

template<typename U>
struct is_input_iterator
{
  enum {
    value = std::is_base_of<
      std::input_iterator_tag,
      typename std::iterator_traits<U>::iterator_category
    >::value
  };
};

template<typename T>
struct is_iterable
{
  typedef char yes;
  typedef char no[2];

  template<typename U>
  static auto check(U*) -> decltype(
    std::enable_if<
      is_input_iterator<decltype(std::begin(declval<U>()))>::value
    >(),
    std::enable_if<
      is_input_iterator<decltype(std::end(declval<U>()))>::value
    >(),
    std::enable_if<
      std::is_same<
        decltype(std::begin(declval<U>())),
        decltype(std::end(declval<U>()))
      >::value
    >(),
    yes()
  );

  template<typename>
  static no& check(...);

  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));
};

// adapted from http://stackoverflow.com/questions/3980879/type-decision-based-on-existence-of-nested-typedef

template <typename T>
struct has_mapped_type
{
  typedef char yes;
  typedef char no[2];

  template <typename C> static yes& check(typename C::mapped_type*);
  template <typename> static no& check(...);
  
  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));
};

template <typename T>
struct has_key_type
{
  typedef char yes;
  typedef char no[2];

  template <typename C> static yes& check(typename C::key_type*);
  template <typename> static no& check(...);
  
  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));
};

template <typename T>
struct has_value_type
{
  typedef char yes;
  typedef char no[2];

  template <typename C> static yes& check(typename C::value_type*);
  template <typename> static no& check(...);
  
  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));
};

// implemented using the above

template <typename T>
struct is_iterable_map
{
  typedef char yes;
  typedef char no[2];

  template <typename C> static yes& check(C*, 
    typename std::enable_if<
      has_mapped_type<C>::value &&
      is_iterable<C>::value
    >::type* dummy = nullptr);
  
  template <typename> static no& check(...);
  
  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));  
};

template <typename T>
struct is_iterable_non_map
{
  typedef char yes;
  typedef char no[2];

  template <typename C> static yes& check(C*, 
    typename std::enable_if<
      !has_mapped_type<C>::value &&
      is_iterable<C>::value
    >::type* dummy = nullptr);
  
  template <typename> static no& check(...);
  
  static const bool value = (sizeof(check<typename std::decay<T>::type>(nullptr)) == sizeof(yes));  
};

} /* util namespace */

#endif
