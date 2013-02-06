#include <mongo/client/dbclient.h>
#include <boost/assert.hpp>
#include "acl/recode/groupdb.hpp"
#include "acl/recode/group.hpp"
#include "db/bson/error.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"
#include "util/verify.hpp"

namespace db { namespace recode
{

bool GroupDB::Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    group.name = obj["name"].String();
    group.id = obj["gid"].Int();
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("group", e, obj);
    return false;
  }
  return true;
}

mongo::Query GroupDB::DefaultQuery()
{
  if (group.id != -1) return QUERY("gid" << group.id);
  else if (!group.name.empty()) return QUERY("name" << group.name);
  else verify(false && "id or name must be set before calling load");
}

bool GroupDB::Load()
{
  boost::unique_future<bool> future;
  std::vector<mongo::BSONObj> results;
  db::Pool::Queue(std::make_shared<db::Select>("groups", DefaultQuery(), results, future));
  if (!future.get() || results.empty()) return false;
  return Unserialize(results[0]);
}

void GroupDB::SaveName()
{
  auto query = QUERY("gid" << group.id);
  auto update = BSON("name" << group.name);
  db::Pool::Queue(std::make_shared<db::Update>("groups", query, update));
}


} /* recode namespace */
} /* db namespace */