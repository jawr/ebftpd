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
