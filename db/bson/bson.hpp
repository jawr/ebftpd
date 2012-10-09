#ifndef __DB_BSON_BSON_HPP
#define __DB_BSON_BSON_HPP

#include <mongo/client/dbclient.h>

namespace db { namespace bson
{

template <typename Container>
mongo::BSONArray SerializeContainer(const Container& c)
{
  mongo::BSONArrayBuilder bab;
  for (const auto& elem : c) bab.append(elem);
  return bab.arr();
}

} /* bson namespace */
} /* db namespace */

#endif
