#include "db/bson/error.hpp"
#include "logs/logs.hpp"

namespace db { namespace bson {

void UnserializeFailure(const std::string& objectName, const std::exception& e, 
    const mongo::BSONObj& bo, bool logOnly)
{
  logs::db << "Error while unserializing " << objectName << ": " 
           << e.what() << ": " << bo.toString() << logs::endl;
           
  if (!logOnly) throw DBUnserializeError(objectName);
}

} /* bson namespace */
} /* db namespace */
