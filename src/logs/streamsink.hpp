#ifndef __LOGS_STREAMSINK_HPP
#define __LOGS_STREAMSINK_HPP

#include <memory>
#include <ostream>
#include <boost/thread/tss.hpp>
#include <boost/thread/mutex.hpp>
#include "logs/sink.hpp"

namespace logs
{

class Stream
{
  std::shared_ptr<boost::mutex> mutex;
  std::shared_ptr<std::ostream> stream;
  bool takeOwnership;
  
public:
  Stream(std::ostream* stream, bool takeOwnership = true) :
    mutex(new boost::mutex())
  {
    if (takeOwnership) this->stream.reset(stream);
    else this->stream.reset(stream, [](void*){});
  }
  
  void Write(const std::string& line)
  {
    boost::lock_guard<boost::mutex> lock(*mutex);
    *stream << line << std::endl;
  }
};

class StreamSink : public Sink
{
  char quoteChar;
  
protected:
  std::vector<Stream> streams;
  boost::thread_specific_ptr<std::ostringstream> buffer;
  
public:
  template <typename... Streams>
  StreamSink(const Stream& stream, const Streams&... streams) :
    quoteChar('\0'),
    streams {stream, streams...}
  {
  }

  void Write(const char* field, int value);
  void Write(const char* field, long long value);
  void Write(const char* field, double value);
  void Write(const char* field, bool value); 
  void Write(const char* field, const char* value);
  void Write(const char* field, const std::string& value)
  { Write(field, value.c_str()); }
  
  void QuoteNext(char quoteChar) { this->quoteChar = quoteChar; }
  
  void Flush();
};

} /* logs namespace */

#endif
