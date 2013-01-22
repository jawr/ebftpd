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
  long long sendBytes;
  long long receiveBytes;
  std::string section;
  
public:
  Traffic() : sendBytes(0), receiveBytes(0) { }
  Traffic(long long sendBytes, long long receiveBytes) : 
    sendBytes(sendBytes), receiveBytes(receiveBytes) { }
  Traffic(long long sendBytes, long long receiveBytes, const std::string& section) : 
    sendBytes(sendBytes), receiveBytes(receiveBytes), section(section)
  { }
  
  long long SendBytes() const { return sendBytes; }
  long long ReceiveBytes() const { return receiveBytes; }
  
  const std::string& Section() const { return section; }
  
  friend struct db::bson::Traffic;
};

} /* stats namespace */
} /* db namespace */

#endif
