#include <ctime>
#include <boost/lexical_cast.hpp>
#include "logs/filesink.hpp"
#include "logs/util.hpp"

namespace logs
{

void FileSink::Write(const char* field, int value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void FileSink::Write(const char* field, long long value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void FileSink::Write(const char* field, double value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void FileSink::Write(const char* field, bool value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void FileSink::Write(const char* /* field */, const char* value)
{
  std::ostringstream* os = buffer.get();
  if (!os)
  {
    os = new std::ostringstream();
    buffer.reset(os);
  }
  else
  if (!os->str().empty())
  {
    *os << ' ';
  }
  
  if (bracketChar.first != '\0') *os << bracketChar.first;
  if (quoteChar != '\0') *os << quoteChar;
  *os << value;
  if (tag) *os << ':';
  if (quoteChar != '\0') *os << quoteChar;
  if (bracketChar.second != '\0') *os << bracketChar.second;
}

void FileSink::Flush()
{
  std::ostringstream* os = buffer.get();
  if (os)
  {
    std::ofstream of(path.c_str(), std::ios::app);
    if (of) of << Timestamp() << ' ' << os->str() << std::endl;
  }
}

} /* logs namespace */
