#include <ctime>
#include <boost/lexical_cast.hpp>
#include "logs/streamsink.hpp"

namespace logs
{

namespace
{
std::string Timestamp()
{
  time_t now = time(nullptr);
  char buf[26];
  strftime(buf,sizeof(buf),"%a %b %d %T %Y", localtime(&now));
  return buf;
}
}

void StreamSink::Write(const char* field, int value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void StreamSink::Write(const char* field, long long value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void StreamSink::Write(const char* field, double value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void StreamSink::Write(const char* field, bool value)
{
  Write(field, boost::lexical_cast<std::string>(value));
}

void StreamSink::Write(const char* /* field */, const char* value)
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

void StreamSink::Flush()
{
  std::ostringstream* os = buffer.get();
  if (os)
  {
    std::string buf(Timestamp());
    buf += ' ';
    buf += os->str();
    for (auto& stream : streams)
    {
      stream.Write(buf);
    }
    os->str(std::string());
  }
}

} /* logs namespace */
