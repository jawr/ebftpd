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
