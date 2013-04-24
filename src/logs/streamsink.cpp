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

#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "logs/streamsink.hpp"
#include "logs/util.hpp"

namespace logs
{

void StreamSink::Write(const char* field, int value)
{
  Write(field, std::to_string(value));
}

void StreamSink::Write(const char* field, long long value)
{
  Write(field, std::to_string(value));
}

void StreamSink::Write(const char* field, double value)
{
  Write(field, std::to_string(value));
}

void StreamSink::Write(const char* field, bool value)
{
  Write(field, static_cast<int>(value));
}

void StreamSink::Write(const char* field, const boost::posix_time::ptime& value)
{
  Write(field, boost::posix_time::to_simple_string(value));
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
