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

#ifndef __DB_LOGSINK_HPP
#define __DB_LOGSINK_HPP

#include <string>
#include <boost/thread/tss.hpp>
#include "logs/sink.hpp"
#include "db/error.hpp"

namespace mongo
{
class BSONObjBuilder;
}

namespace db
{

struct LogCreationError : public DBError
{
  LogCreationError() : std::runtime_error("Failed to create log database collection") { }
};

class LogSink : public logs::Sink
{
  std::string collection;
  boost::thread_specific_ptr<mongo::BSONObjBuilder> buffer;

  mongo::BSONObjBuilder* Builder();

  void CreateCollection(long size);
  
public:
  LogSink(const std::string& collection, long size) :
    collection(collection)
  {
    CreateCollection(size);
  }

  void Write(const char* field, int value);
  void Write(const char* field, long long value);
  void Write(const char* field, double value);
  void Write(const char* field, bool value);
  void Write(const char* field, const char* value);
  void Write(const char* field, const std::string& value) { Write(field, value.c_str()); }
  void Write(const char* field, const boost::posix_time::ptime& value);
  
  void Flush();
};

} /* db namespace */

#endif
