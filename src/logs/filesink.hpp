#ifndef __LOGS_FILESINK_HPP
#define __LOGS_FILESINK_HPP

#include <memory>
#include <ostream>
#include <boost/thread/tss.hpp>
#include <mutex>
#include "logs/sink.hpp"

namespace logs
{

class FileSink : public Sink
{
  char quoteChar;
  bool tag;
  std::pair<char, char> bracketChar;
  
protected:
  std::string path;
  boost::thread_specific_ptr<std::ostringstream> buffer;
  
public:
  FileSink(const std::string& path) :
    quoteChar('\0'),
    tag(false),
    bracketChar{'\0', '\0'},
    path(path)
  {
  }

  void Write(const char* field, int value);
  void Write(const char* field, long long value);
  void Write(const char* field, double value);
  void Write(const char* field, bool value); 
  void Write(const char* field, const char* value);
  void Write(const char* field, const std::string& value)
  { Write(field, value.c_str()); }
  
  void Formatting(bool tag, char quoteChar, const std::pair<char, char>& bracketChar)
  {
    this->tag = tag;
    this->quoteChar = quoteChar;
    this->bracketChar = bracketChar;
  }
  
  void Flush();
};

} /* logs namespace */

#endif
