#include <cassert>
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
  bob.append("status", db::mail::StatusToString(message.status));
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
    message.status = db::mail::StatusFromString(bo["status"].String());
    mongo::BSONElement oid;
    bo.getObjectID(oid);
    message.oid = oid.OID();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("message", e, bo);
  }
  catch (const std::out_of_range& e)
  {
    UnserializeFailure("message", e, bo);
  }
  return message;
}

} /* bson namespace */
} /* db namespace */