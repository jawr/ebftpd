#ifndef __DB_LOGSINK_HPP
#define __DB_LOGSINK_HPP

#include <string>
#include <boost/thread/tss.hpp>
#include "logs/sink.hpp"

namespace mongo
{
class BSONObjBuilder;
}

namespace db
{

class LogSink : public logs::Sink
{
  std::string collection;
  boost::thread_specific_ptr<mongo::BSONObjBuilder> buffer;

  mongo::BSONObjBuilder* Builder();
  
public:
  LogSink(const std::string& collection) :
    collection(collection)
  { }

  void Write(const char* field, int value);
  void Write(const char* field, long long value);
  void Write(const char* field, double value);
  void Write(const char* field, bool value);
  void Write(const char* field, const char* value);
  void QuoteNext(char /* quoteChar */) { }
  
  void Flush();
};

} /* db namespace */

#endif
