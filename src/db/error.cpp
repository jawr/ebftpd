#include "db/error.hpp"
#include "logs/logs.hpp"

namespace db
{

void IDGenerationFailure(const std::string& type, const mongo::DBException& e)
{
  logs::db << "Error while generating " << type << " id: " << e.what() << logs::endl;
  throw DBError("Unable to generate new " + type + " id.");
}

} /* db namespace */
