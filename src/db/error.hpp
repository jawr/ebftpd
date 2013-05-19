//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __DB_ERROR_HPP
#define __DB_ERROR_HPP

#include <mongo/client/dbclient.h>
#include <sstream>
#include <typeinfo>
#include "logs/logs.hpp"
#include "util/error.hpp"
#include "util/debug.hpp"

namespace db
{

struct LastError : public mongo::BSONObj
{
  explicit LastError(const mongo::BSONObj& errObj) : mongo::BSONObj(errObj) { }
  bool Okay() const { return (*this)["err"].isNull(); }
  int NumChanged() const { return (*this)["n"].Int(); }
};

struct DBError : public util::RuntimeError
{
  DBError() : std::runtime_error("Unknown database error") { }
  DBError(const std::string& message) : std::runtime_error(message) { }
};

struct DBWriteError : public DBError
{
  DBWriteError() : std::runtime_error("Error while writing to database") { }
  DBWriteError(const std::string& message) : std::runtime_error(message) { }
};

struct DBReadError : public DBError
{
  DBReadError() : std::runtime_error("Error while querying the database") { }
  DBReadError(const std::string& message) : std::runtime_error(message) { }
};

struct DBKeyError : public DBError
{
  DBKeyError() : std::runtime_error("Duplicate key error") { }
  DBKeyError(const std::string& message) : std::runtime_error(message) { }
};

inline void LogError(std::ostringstream& os, const std::string& errmsg)
{
  os << " : " << errmsg;
  logs::Database(os.str());
}

inline std::ostream& operator<<(std::ostream& os, const std::type_info& info)
{
  return (os << util::debug::Demangle(info.name()));
}

template <typename T, typename... Args>
void LogError(std::ostringstream& os, const std::string& errmsg, const T& arg, const Args&... args)
{
  os << " : " << arg;
  LogError(os, errmsg, args...);
}

template <typename... Args>
void LogError(const std::string& prefix, const std::string& errmsg, const Args&... args)
{
  std::ostringstream buf;
  buf << std::boolalpha << prefix << " failed";
  LogError(buf, errmsg, args...);
}

template <typename... Args>
void LogException(const std::string& prefix, const std::exception& e, const Args&... args)
{
  LogError(prefix, e.what(), args...);
}

template <typename... Args>
void LogLastError(const std::string& prefix, const mongo::BSONObj& obj, const Args&... args)
{
  LogError(prefix, obj["err"].String(), args...);
}

inline void LastErrorToException(const mongo::BSONObj& obj)
{
  if (!obj["err"].isNull()) throw mongo::DBException(obj["err"].String(), obj["code"].Int());
}

inline void LastErrorToException(mongo::DBClientBase& conn)
{
  LastErrorToException(conn.getLastErrorDetailed());
}


} /* db namespace */

#endif
