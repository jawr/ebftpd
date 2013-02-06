#include "acl/recode/userdb.hpp"

namespace db { namespace recode
{

bool UserDB::Unserialize(const mongo::BSONObj& obj)
{
  try
  {
    user.modified = ToPosixTime(obj["modified"].Date());
    user.id = obj["id"].Int();
    user.name = obj["name"].String();
    auto ipMasks = obj["ip masks"].Array();
    for (const auto& el : ipMasks)
      user.ipMasks.emplace_back(el.String());
  }
  catch (const mongo::DBException& e)
  {
    db::bson::UnserializeFailure("user", e, bo);
    return false;
  }
  return true;
}

mongo::Query UserDB::DefaultQuery()
{
  if (user.id != -1) return QUERY("gid" << user.id);
  else if (!user.name.empty()) return QUERY("name" << user.name);
  else verify(false && "id or name must be set before calling load");
}

bool UserDB::Load()
{
  boost::unique_future<bool> future;
  std::vector<mongo::BSONObj> results;
  db::Pool::Queue(std::make_shared<db::Select>("users", DefaultQuery(), results, future));
  if (!future.get() || results.empty()) return false;
  return Unserialize(results[0]);
}

void UserDB::SaveName()
{
  std::cout << "SaveName" << std::endl;
}

void UserDB::SaveIPMasks()
{
}

} /* recode namespace */
} /* db namespace */
