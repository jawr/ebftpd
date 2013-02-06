#include <mongo/client/dbclient.h>
#include "acl/recode/groupprofiledb.hpp"
#include "acl/recode/groupprofile.hpp"
#include "util/verify.hpp"
#include "db/bson/error.hpp"
#include "db/task.hpp"
#include "db/pool.hpp"

namespace db { namespace recode
{

mongo::BSONObj GroupProfileDB::Serialize()
{
  mongo::BSONObjBuilder bob;
  bob.append("name", profile.name);
  bob.append("gid", profile.id);
  bob.append("description", profile.description);
  bob.append("slots", profile.slots);
  bob.append("leech slots", profile.leechSlots);
  bob.append("allotment slots", profile.allotmentSlots);
  bob.append("max allotment size", profile.maxAllotmentSize);
  bob.append("max logins", profile.maxLogins);
  bob.append("comment", profile.comment);
  return bob.obj();

}

bool GroupProfileDB::Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    profile.name = obj["name"].String();
    profile.id = obj["gid"].Int();
    profile.description = obj["description"].String();
    profile.comment = obj["comment"].String();
    profile.slots = obj["slots"].Int();
    profile.leechSlots = obj["leech slots"].Int();
    profile.allotmentSlots = obj["allotment slots"].Int();
    profile.maxAllotmentSize = obj["max allotment size"].Long();
    profile.slots = obj["slots"].Int();
    profile.maxLogins = obj["max logins"].Int();
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("group", e, obj);
    return false;
  }
  return true;
}

mongo::Query GroupProfileDB::DefaultQuery()
{
  if (profile.id != -1) return QUERY("gid" << profile.id);
  else if (!profile.name.empty()) return QUERY("name" << profile.name);
  else verify(false && "id or name must be set before calling load");
}

bool GroupProfileDB::Load()
{
  boost::unique_future<bool> future;
  std::vector<mongo::BSONObj> results;
  db::Pool::Queue(std::make_shared<db::Select>("groups", DefaultQuery(), results, future));
  if (!future.get() || results.empty()) return false;
  return Unserialize(results[0]);
}

void GroupProfileDB::Save()
{
  db::Pool::Queue(std::make_shared<db::Update>("groups", DefaultQuery(), Serialize(), true));
}

void GroupProfileDB::SaveField(const std::string& field)
{
  auto obj = Serialize();
  auto update = BSON("$set" << BSON(field << obj[field]) <<
                     "$set" << BSON("modified" << profile.modified));
  db::Pool::Queue(std::make_shared<db::Update>("groups", DefaultQuery(), update, false));
}

void GroupProfileDB::SaveDescription() { SaveField("description"); }
void GroupProfileDB::SaveComment() { SaveField("comment"); }
void GroupProfileDB::SaveSlots() { SaveField("slots"); }
void GroupProfileDB::SaveLeechSlots() { SaveField("leech slots"); }
void GroupProfileDB::SaveAllotmentSlots() { SaveField("allotment slots"); }
void GroupProfileDB::SaveMaxAllotmentSize() { SaveField("max allotment size"); }
void GroupProfileDB::SaveMaxLogins() { SaveField("max logins"); }

} /* recode namespace */
} /* acl namespace */
