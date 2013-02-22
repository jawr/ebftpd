#ifndef __LOGS_SINK_HPP
#define __LOGS_SINK_HPP

#include <string>

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
  void Write(const char* field, const std::string& value) { Write(field, value.c_str()); }
  virtual void QuoteNext(char quoteChar) = 0;

  virtual void Flush() = 0;
};

} /* logs namespace */

#endif
