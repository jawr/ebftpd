#include <boost/optional.hpp>
#include "db/bson/message.hpp"
#include "db/bson/bson.hpp"
#include "db/mail/message.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

mongo::BSONObj Message::Serialize(const db::mail::Message& message)
{
  mongo::BSONObjBuilder bob;
  bob.append("recipient", message.recipient);
  bob.append("sender", message.sender);
  bob.append("time sent", ToDateT(message.timeSent));
  bob.append("body", message.body);
  bob.append("trash", message.trash);
  return bob.obj();
}

db::mail::Message Message::Unserialize(const mongo::BSONObj& bo)
{
  db::mail::Message message;
  try
  {
    message.recipient = bo["recipient"].Int();
    message.sender = bo["sender"].String();
    message.timeSent = ToPosixTime(bo["time sent"].Date());
    message.body = bo["body"].String();
    message.trash = bo["trash"].Bool();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("message", e, bo);
  }
  return message;
}

} /* bson namespace */
} /* db namespace */