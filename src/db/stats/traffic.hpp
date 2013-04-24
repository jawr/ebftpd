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

#ifndef __DB_STATS_TRAFFIC_HPP
#define __DB_STATS_TRAFFIC_HPP

namespace db 
{ 

namespace bson
{
class Traffic;
}

namespace stats
{

class Traffic
{
private:
  long long sendKBytes;
  long long receiveKBytes;
  std::string section;
  
public:
  Traffic() : sendKBytes(0), receiveKBytes(0) { }
  Traffic(long long sendKBytes, long long receiveKBytes) : 
    sendKBytes(sendKBytes), receiveKBytes(receiveKBytes) { }
  Traffic(long long sendKBytes, long long receiveKBytes, const std::string& section) : 
    sendKBytes(sendKBytes), receiveKBytes(receiveKBytes), section(section)
  { }
  
  long long SendKBytes() const { return sendKBytes; }
  long long ReceiveKBytes() const { return receiveKBytes; }
  
  const std::string& Section() const { return section; }
  
  friend class db::bson::Traffic;
};

} /* stats namespace */
} /* db namespace */

#endif
