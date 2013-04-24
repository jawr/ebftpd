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
