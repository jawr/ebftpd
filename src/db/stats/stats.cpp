#include <boost/date_time/posix_time/posix_time.hpp>
#include <mongo/client/dbclient.h>
#include "db/stats/stats.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "db/types.hpp"

namespace db { namespace stats
{

void UploadDecr(const acl::User& user, long long kbytes)
{
  using namespace boost::posix_time;
  auto now = second_clock::local_time();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << now.date().day()
    << "week" << now.date().week_number() << "month" 
    << now.date().month().as_number() << "year" << now.date().year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << -1) <<
    "$inc" << BSON("kbytes" << kbytes*-1));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Upload(const acl::User& user, long long kbytes, double xfertime)
{
  using namespace boost::posix_time;
  auto now = second_clock::local_time();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << now.date().day()
    << "week" << now.date().week_number() << "month" 
    << now.date().month().as_number() << "year" << now.date().year()
    << "direction" << "up");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("kbytes" << kbytes) <<
    "$inc" << BSON("xfertime" << xfertime));
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

void Download(const acl::User& user, long long kbytes, double xfertime)
{
  using namespace boost::posix_time;
  auto now = second_clock::local_time();
  mongo::Query query = QUERY("uid" << user.UID() << "day" << now.date().day()
    << "week" << now.date().week_number() << "month" 
    << now.date().month().as_number() << "year" << now.date().year()
    << "direction" << "dn");
  mongo::BSONObj obj = BSON(
    "$inc" << BSON("files" << 1) <<
    "$inc" << BSON("kbytes" << kbytes*-1) <<
    "$inc" << BSON("xfertime" << xfertime)); // how to handle the xfertime
  TaskPtr task(new db::Update("transfers", query, obj, true));
  Pool::Queue(task);
}

// end
}
}
