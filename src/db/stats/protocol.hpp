#ifndef __DB_STATS_PROTOCOL_HPP
#define __DB_STATS_PROTOCL_HPP

#include "acl/types.hpp"

namespace db { namespace bson
{
struct Protocol;
}

namespace stats
{

class Protocol
{
  acl::UserID uid;
  
  int day;
  int week;
  int month;
  int year;
  
  long long sendBytes;
  long long receiveBytes;
  
public:
  Protocol() : sendBytes(0), receiveBytes(0) { }
  acl::UserID UID() const { return uid; }
  
  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
  
  long long SendBytes() const { return sendBytes; }
  long long ReceiveBytes() const { return receiveBytes; }
  
  friend struct db::bson::Protocol;
};

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes);

} /* stats namespace */
} /* db namespace */

#endif
