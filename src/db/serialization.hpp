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

#ifndef __DB_SERIALIZATION_HPP
#define __DB_SERIALIZATION_HPP

#include <cstdint>
#include <set>
#include <ctime>
#include <mongo/client/dbclient.h>
#include "util/typetraits.hpp"
#include "util/verify.hpp"

namespace db
{

template <typename T>
mongo::BSONObj Serialize(const T& /* obj */)
{
  static_assert(!std::is_same<T, T>::value, "You must specify an explicit specialization");
  return mongo::BSONObj();
}

template <typename T>
T Unserialize(const mongo::BSONObj& /* obj */)
{
  static_assert(!std::is_same<T, T>::value, "You must specify an explicit specialization");
  return T();
}

template <typename Container>
mongo::BSONArray SerializeContainer(const Container& c, 
  typename std::enable_if<util::is_iterable_non_map<Container>::value>::type* dummy = nullptr)
{
  mongo::BSONArrayBuilder bab;
  for (auto& elem : c)
    bab.append(elem);
  return bab.arr();
  
  (void) dummy;
}

template <typename Container>
mongo::BSONArray SerializeContainer(const Container& c,
  typename std::enable_if<util::is_iterable_map<Container>::value>::type* dummy = nullptr)
{
  mongo::BSONArrayBuilder bab;
  for (auto& kv : c)
    bab.append(BSON(kv.first << kv.second));
  return bab.arr();
  
  (void) dummy;
}

template <typename Container>
void UnserializeContainer(const std::vector<mongo::BSONElement>& arr, Container& c,
  typename std::enable_if<util::is_iterable_non_map<Container>::value &&
                          !util::has_key_type<Container>::value>::type* dummy = nullptr)
{
  c.clear();
  for (const auto& elem : arr)
  {
    typename Container::value_type value;
    elem.Val(value);
    c.push_back(value);
  }
  
  (void) dummy;
}

template <typename Container>
void UnserializeContainer(const std::vector<mongo::BSONElement>& arr, Container& c,
  typename std::enable_if<util::is_iterable_non_map<Container>::value &&
                          util::has_key_type<Container>::value>::type* dummy = nullptr)
{
  c.clear();
  for (const auto& elem : arr)
  {
    typename Container::value_type value;
    elem.Val(value);
    c.insert(value);
  }
  
  (void) dummy;
}

template <typename Map>
mongo::BSONArray SerializeMap(const Map& map, 
      const std::string& keyField, const std::string& valueField)
{
  mongo::BSONArrayBuilder bab;
  for (const auto& kv : map)
  {
    bab.append(BSON(keyField << kv.first << valueField << kv.second));
  }
  return bab.arr();
}

template <typename Map>
void UnserializeMap(const std::vector<mongo::BSONElement>& arr, 
      const std::string& keyField, const std::string& valueField, Map& map)
{
  map.clear();
  for (const auto& elem : arr)
  {
    auto elemObj = elem.Obj();
 
    typename Map::key_type key;
    elemObj[keyField].Val(key);
      
    typename Map::mapped_type value;
    elemObj[valueField].Val(value);
    
    map.insert(std::make_pair(key, value));
  }
}

// very very ugly hack!!
template <> inline int32_t Unserialize<int32_t>(const mongo::BSONObj& obj)
{
  std::set<std::string> fields;
  obj.getFieldNames(fields);
  if (fields.find("uid") != fields.end()) return obj["uid"].Int();
  else return obj["gid"].Int();
}

mongo::Date_t ToDateT(const boost::posix_time::ptime& t);
inline mongo::Date_t ToDateT(const boost::gregorian::date& d)
{ return ToDateT(boost::posix_time::ptime(d)); }

inline mongo::Date_t ToDateT(time_t t)
{ return mongo::Date_t(t * 1000); }

boost::posix_time::ptime ToPosixTime(const mongo::Date_t& dt);
inline boost::gregorian::date ToGregDate(const mongo::Date_t& dt)
{ return ToPosixTime(dt).date(); }

} /* db namespace */

#endif
