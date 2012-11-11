#include "db/stats/protocol.hpp"
#include "db/stats/date.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"

namespace db { namespace stats
{

void ProtocolUpdate(acl::UserID uid, long long sendBytes, long long receiveBytes)
{
  Date date;
  
  mongo::BSONObjBuilder qbob;
  qbob.append("uid", uid);
  qbob.append("day", date.Day());
  qbob.append("week", date.Week());
  qbob.append("month", date.Month());
  qbob.append("year", date.Year());
  mongo::Query query(qbob.obj());
  
  mongo::BSONObj bo = BSON("$inc" << BSON("send bytes" << sendBytes) <<
                           "$inc" << BSON("receive bytes" << receiveBytes));
  TaskPtr task(new db::Update("protocol", query, bo, true));
  Pool::Queue(task);
}

} /* stats namespace */
} /* db namespace */