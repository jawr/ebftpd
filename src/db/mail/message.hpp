#ifndef __DB_MAIL_MESSAGE_HPP
#define __DB_MAIL_MESSAGE_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <mongo/client/dbclient.h>
#include "acl/types.hpp"
#include "util/enumstrings.hpp"

namespace db
{

namespace mail
{
class Message;
}

template <typename T> mongo::BSONObj Serialize(const mail::Message& message);
template <> mongo::BSONObj Serialize<mail::Message>(const mail::Message& message);

template <typename T> T Unserialize(const mongo::BSONObj& obj);
template <> mail::Message Unserialize<mail::Message>(const mongo::BSONObj& obj);

namespace mail
{

enum class Status : unsigned { Unread, Trash, Saved };

class Message
{
private:
  std::string sender;
  acl::UserID recipient;
  std::string body;
  boost::posix_time::ptime timeSent;
  ::db::mail::Status status;
  mongo::OID oid;
  
  Message() : recipient(-1), status(db::mail::Status::Unread) { }
  
public:
  Message(const std::string& sender, acl::UserID recipient, 
          const std::string& body, 
          boost::posix_time::ptime& timeSent) :
    sender(sender), recipient(recipient),
    body(body), timeSent(timeSent), status(db::mail::Status::Unread) { }
  
  const std::string& Sender() const { return sender; }
  acl::UserID Recipient() const { return recipient; }
  const std::string& Body() const { return body; }
  const boost::posix_time::ptime& TimeSent() const { return timeSent; }
  ::db::mail::Status Status() const { return status; }
  
  friend void Trash(const Message& message);
  friend Message db::Unserialize<Message>(const mongo::BSONObj& obj);
  friend mongo::BSONObj db::Serialize<Message>(const Message& message);
};

std::string StatusToString(Status status);
Status StatusFromString(const std::string& status);

} /* mail namespace */
} /* db namespace */

namespace util
{
template <> const char* util::EnumStrings<db::mail::Status>::values[];
}

#endif
