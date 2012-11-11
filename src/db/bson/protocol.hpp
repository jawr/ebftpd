#ifndef __DB_BSON_PROTOCOL_HPP
#define __DB_BSON_PROTCOL_HPP

#include <mongo/client/dbclient.h>
#include "db/stats/protocol.hpp"

namespace db { namespace bson
{

struct Protocol
{
  static mongo::BSONObj Serialize(const db::stats::Protocol& proto);
  static db::stats::Protocol Unserialize(const mongo::BSONObj& bo);
};

} /* bson namespace */
} /* db namespace */

#endif
