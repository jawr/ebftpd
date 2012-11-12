#ifndef __DB_STATS_PROTOCOL_HPP
#define __DB_STATS_PROTOCOL_HPP

#include "acl/types.hpp"
#include "stats/types.hpp"

namespace db { namespace bson
{
struct Protocol;
}

namespace stats
{
class Protocol
{
private:
  long long sendBytes;
  long long receiveBytes;
  
public:
  Protocol() : sendBytes(0), receiveBytes(0) { }
  Protocol(long long sendBytes, long long receiveBytes) : 
    sendBytes(sendBytes), receiveBytes(receiveBytes) { }
  
  long long SendBytes() const { return sendBytes; }
  long long ReceiveBytes() const { return receiveBytes; }
  
  friend struct db::bson::Protocol;
};

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes);
Protocol CalculateUser(acl::UserID uid, ::stats::Timeframe timeframe);
Protocol CalculateTotal(::stats::Timeframe timeframe);

} /* stats namespace */
} /* db namespace */

#endif
