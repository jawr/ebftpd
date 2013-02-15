#ifndef __DB_SERIALIZATION_HPP
#define __DB_SERIALIZATION_HPP

#include <mongo/client/dbclient.h>
#include "util/typetraits.hpp"
#include "util/verify.hpp"

namespace db
{

template <typename Container>
mongo::BSONArray Serialize(const Container& c, 
  typename std::enable_if<util::is_iterable_non_map<Container>::value>::type* dummy = nullptr)
{
  mongo::BSONArrayBuilder bab;
  for (auto& elem : c)
    bab.append(elem);
  return bab.arr();
}

template <typename Container>
mongo::BSONArray Serialize(const Container& c,
  typename std::enable_if<util::is_iterable_map<Container>::value>::type* dummy = nullptr)
{
  mongo::BSONArrayBuilder bab;
  for (auto& kv : c)
    bab.append(BSON(kv.first << kv.second));
  return bab.arr();
}

template <typename T> T Unserialize(const mongo::BSONObj& obj);

template <typename Container>
Container Unserialize(const std::vector<mongo::BSONElement>& arr,
  typename std::enable_if<util::is_iterable_non_map<Container>::value &&
                          !util::has_key_type<Container>::value>::type* dummy = nullptr)
{
  Container c;
  for (const auto& elem : arr)
  {
    typename Container::value_type value;
    elem.Val(value);
    c.push_back(value);
  }
  return c;
}

template <typename Container>
Container Unserialize(const std::vector<mongo::BSONElement>& arr,
  typename std::enable_if<util::is_iterable_non_map<Container>::value &&
                          util::has_key_type<Container>::value>::type* dummy = nullptr)
{
  Container c;
  for (const auto& elem : arr)
  {
    typename Container::value_type value;
    elem.Val(value);
    c.insert(value);
  }
  return c;
}

template <typename Container>
Container Unserialize(const std::vector<mongo::BSONElement>& arr,
  typename std::enable_if<util::is_iterable_map<Container>::value &&
                          std::is_same<std::string, typename Container::key_type>::value>::type* dummy = nullptr)
{
  Container c;
  for (const auto& elem : arr)
  {
    auto elemObj = elem.Obj();
    std::set<std::string> fieldNames;
    elemObj.getFieldNames(fieldNames);
    verify(fieldNames.size() == 1); // should be exception
    const std::string& key = *fieldNames.begin();
    
    typename Container::mapped_type value;
    elemObj[key].Val(value);
    
    c.insert(std::make_pair(key, value));
  }
  return c;
}

mongo::Date_t ToDateT(const boost::posix_time::ptime& t);
inline mongo::Date_t ToDateT(const boost::gregorian::date& d)
{ return ToDateT(boost::posix_time::ptime(d)); }

boost::posix_time::ptime ToPosixTime(const mongo::Date_t& dt);
inline boost::gregorian::date ToGregDate(const mongo::Date_t& dt)
{ return ToPosixTime(dt).date(); }

} /* db namespace */

#endif
