#include "db/bson/error.hpp"
#include "logs/logs.hpp"

namespace db { namespace bson {

void UnserializeFailure(const std::string& objectName, const mongo::DBException& e, const mongo::BSONObj& bo)
{
  logs::db << "Error while unserializing " << objectName << ": " 
           << e.what() << ": " << bo.toString() << logs::endl;
           
  throw DBUnserializeError(objectName);
}

} /* bson namespace */
} /* db namespace */
