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

#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/logsink.hpp"
#include "db/connection.hpp"

namespace db
{

void LogSink::CreateCollection(long size)
{
  try
  {
    SafeConnection conn;
    mongo::BSONObj info;
    conn.RunCommand(BSON("create" << collection << 
                         "capped" << true << 
                         "size" << static_cast<long long>(size * 1024)), info);
    return;
  }
  catch (const mongo::DBException&)
  { }
  catch (const DBError&)
  { }
  
  throw LogCreationError();
}

mongo::BSONObjBuilder* LogSink::Builder()
{
  mongo::BSONObjBuilder* bab = buffer.get();
  if (!bab)
  {
    bab = new mongo::BSONObjBuilder();
    buffer.reset(bab);
  }
  return bab;
}

void LogSink::Write(const char* field, int value)
{
  Builder()->append(field, value);
}

void LogSink::Write(const char* field, long long value)
{
  Builder()->append(field, value);
}

void LogSink::Write(const char* field, double value)
{
  Builder()->append(field, value);
}

void LogSink::Write(const char* field, bool value)
{
  Builder()->append(field, value);
}

void LogSink::Write(const char* field, const char* value)
{
  Builder()->append(field, value);
}

void LogSink::Write(const char* field, const boost::posix_time::ptime& value)
{
  Builder()->append(field, ToDateT(value));
}

void LogSink::Flush()
{
  mongo::BSONObjBuilder* bab = buffer.get();
  if (bab)
  {
    FastConnection conn;
    bab->append("timestamp", ToDateT(boost::posix_time::microsec_clock::local_time()));
    conn.Insert(collection, bab->obj());
    buffer.reset();
  }
}

} /* db namespace */
