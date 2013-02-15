#ifndef __DB_ERROR_HPP
#define __DB_ERROR_HPP

#include <mongo/client/dbclient.h>
#include <sstream>
#include "logs/logs.hpp"
#include "util/error.hpp"

namespace db
{

struct DBError : public util::RuntimeError
{
  DBError() : std::runtime_error("Unknown database error") { }
  DBError(const std::string& message) : std::runtime_error(message) { }
};

struct DBWriteError : public util::RuntimeError
{
  DBWriteError() : std::runtime_error("Error while writing to database") { }
};

struct DBReadError : public util::RuntimeError
{
  DBReadError() : std::runtime_error("Error while querying the database") { }
};

void LogException(std::ostringstream& os, const mongo::DBException& e)
{
  os << " : " << e.what();
  logs::db << os.str() << logs::endl;
}

template <typename T, typename... Args>
void LogException(std::ostringstream& os, const mongo::DBException& e, const T& arg, Args... args)
{
  os << " : " << arg;
  LogException(os, e, arg, args...);
}

template <typename T, typename... Args>
void LogException(const std::string& command, const mongo::DBException& e, const T& arg, Args... args)
{
  std::ostringstream buf;
  buf << std::boolalpha << command << " failed: " << arg;
  LogException(buf, e, args...);
}

} /* db namespace */

#endif
