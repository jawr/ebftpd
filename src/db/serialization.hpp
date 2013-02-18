#ifndef __DB_SERIALIZATION_HPP
#define __DB_SERIALIZATION_HPP

#include <cstdint>
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
Container UnserializeContainer(const std::vector<mongo::BSONElement>& arr,
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
  
  (void) dummy;
}

template <typename Container>
Container UnserializeContainer(const std::vector<mongo::BSONElement>& arr,
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
  
  (void) dummy;
}

template <typename Container>
Container UnserializeContainer(const std::vector<mongo::BSONElement>& arr,
  typename std::enable_if<util::is_iterable_map<Container>::value &&
                          std::is_same<std::string, typename Container::key_type>::value>::type* dummy = nullptr)
{
  Container c;
  for (const auto& elem : arr)
  {
    auto elemObj = elem.Obj();
    std::set<std::string> fieldNames;
    elemObj.getFieldNames(fieldNames);
    
    if (fieldNames.size() != 1)
      throw mongo::DBException("invalid bson object for mapped container element", 13111);

    const std::string& key = *fieldNames.begin();
    
    typename Container::mapped_type value;
    elemObj[key].Val(value);
    
    c.insert(std::make_pair(key, value));
  }
  return c;
  
  (void) dummy;
}

// very very ugly hack!!
template <> inline int32_t Unserialize<int32_t>(const mongo::BSONObj& obj)
{
  try
  {
    return obj["uid"].Int();
  }
  catch (const mongo::DBException& e)
  {
    if (e.getCode() != 13111) throw e;
    return obj["gid"].Int();
  }
}

mongo::Date_t ToDateT(const boost::posix_time::ptime& t);
inline mongo::Date_t ToDateT(const boost::gregorian::date& d)
{ return ToDateT(boost::posix_time::ptime(d)); }

boost::posix_time::ptime ToPosixTime(const mongo::Date_t& dt);
inline boost::gregorian::date ToGregDate(const mongo::Date_t& dt)
{ return ToPosixTime(dt).date(); }

} /* db namespace */

#endif
