#ifndef __DB_EXCEPTION_HPP
#define __DB_EXCEPTION_HPP

#include <mongo/client/dbclient.h>
#include "util/error.hpp"

namespace db
{

struct DBError : public util::RuntimeError
{
  DBError() : std::runtime_error("Unknown DB error.") { }
  DBError(const std::string& message) : std::runtime_error(message) { }
};

void IDGenerationFailure(const std::string& type, const mongo::DBException& e);

inline void LastErrorToException(const mongo::BSONObj& bo)
{
  if (bo["ok"].Double() != 1.0) throw mongo::DBException(bo["err"].String(), bo["n"].Int());
}

inline void LastErrorToException(mongo::DBClientConnection& conn)
{
  LastErrorToException(conn.getLastErrorDetailed());
}

} /* db namespace */

#endif
