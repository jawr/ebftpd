#ifndef __DB_BSON_BSON_HPP
#define __DB_BSON_BSON_HPP

#include <mongo/client/dbclient.h>

namespace db { namespace bson
{

template <typename Container>
mongo::BSONArray SerializeContainer(const Container& c)
{
  typedef typename Container::const_iterator const_iterator;

  mongo::BSONArrayBuilder bab;
  for (const_iterator it = c.begin(); it != c.end(); ++it)
    bab.append(*it);

  return bab.arr();
}

} /* bson namespace */
} /* db namespace */

#endif
