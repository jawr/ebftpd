#ifndef __LOGS_SINK_HPP
#define __LOGS_SINK_HPP

#include <string>
#include <utility>

namespace boost { namespace posix_time
{
class ptime;
}
}

namespace logs
{

struct Sink
{
  virtual ~Sink() { }
  virtual void Write(const char* field, int value) = 0;  
  virtual void Write(const char* field, long long value) = 0;
  virtual void Write(const char* field, double value) = 0;
  virtual void Write(const char* field, bool value) = 0;
  virtual void Write(const char* field, const char* value) = 0;
  virtual void Write(const char* field, const std::string& value) = 0;
  virtual void Write(const char* field, const boost::posix_time::ptime& value) = 0;
  virtual void Formatting(bool /* tag */, char /* quoteChar */, 
                          const std::pair<char, char>& /* bracketChar */) { }

  virtual void Flush() = 0;
};

} /* logs namespace */

#endif
