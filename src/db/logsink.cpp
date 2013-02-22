#include <boost/date_time/posix_time/posix_time.hpp>
#include "db/logsink.hpp"
#include "db/connection.hpp"

namespace db
{

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
