#ifndef __DB_BSON_ERROR_HPP
#define __DB_BSON_ERROR_HPP

#include <string>
#include <mongo/client/dbclient.h>
#include "db/error.hpp"

namespace db { namespace bson
{

struct DBUnserializeError : public DBError
{
  DBUnserializeError(const std::string& objectName) :
    std::runtime_error("Unable to unserialize " + objectName + " from database.") { }
};

void UnserializeFailure(const std::string& objectName, const std::exception& e, 
    const mongo::BSONObj& bo, bool logOnly = false);

} /* bson namespace */
} /* db namespace */

#endif
