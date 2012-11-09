#ifndef __DB_MAIL_MESSAGE_HPP
#define __DB_MAIL_MESSAGE_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"

namespace db { namespace bson
{
struct Message;
}

namespace mail
{

class Message
{
  std::string sender;
  acl::UserID recipient;
  std::string body;
  boost::posix_time::ptime timeSent;
  bool trash;
  
  Message() : recipient(-1), trash(false) { }
  
public:
  Message(const std::string& sender, acl::UserID recipient, 
          const std::string& body, 
          boost::posix_time::ptime& timeSent) :
    sender(sender), recipient(recipient),
    body(body), timeSent(timeSent), trash(false) { }
  
  const std::string& Sender() const { return sender; }
  acl::UserID Recipient() const { return recipient; }
  const std::string& Body() const { return body; }
  const boost::posix_time::ptime& TimeSent() const { return timeSent; }
  bool Trash() const { return trash; }
  
  
  friend struct db::bson::Message;
};

} /* mail namespace */
} /* db namespace */

#endif
