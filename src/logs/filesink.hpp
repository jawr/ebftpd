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
  void Write(const char* field, const std::string& value) { Write(field, value.c_str()); }
  void Write(const char* field, const boost::posix_time::ptime& value);
  
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
