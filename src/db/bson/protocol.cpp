#include "db/bson/protocol.hpp"
#include "db/bson/error.hpp"

namespace db { namespace bson
{

mongo::BSONObj Protocol::Serialize(const db::stats::Protocol& proto)
{
  mongo::BSONObjBuilder bob;
  bob.append("uid", proto.uid);
  bob.append("day", proto.day);
  bob.append("week", proto.week);
  bob.append("month", proto.month);
  bob.append("year", proto.year);
  bob.append("send bytes", proto.sendBytes);
  bob.append("receive bytes", proto.receiveBytes);
  return bob.obj();
}

db::stats::Protocol Protocol::Unserialize(const mongo::BSONObj& bo)
{
  db::stats::Protocol proto;
  try
  {
    proto.uid = bo["uid"].Int();
    proto.day = bo["day"].Int();
    proto.week = bo["week"].Int();
    proto.month = bo["month"].Int();
    proto.year = bo["year"].Int();
    proto.sendBytes = bo["send bytes"].Long();
    proto.receiveBytes = bo["receive bytes"].Long();
  }
  catch (const mongo::DBException& e)
  {
    UnserializeFailure("protocol", e, bo);
  }
  return proto;
}

} /* bson namespace */
} /* db namespace */
