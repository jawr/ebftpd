#include "db/group/group.hpp"
#include "db/types.hpp"
#include "db/pool.hpp"
#include "db/task.hpp"
#include "db/bson/group.hpp"
#include "acl/group.hpp"
#include "acl/types.hpp"
#include "db/error.hpp"
#include "db/bson/bson.hpp"

namespace db { namespace group
{

bool Create(acl::Group& group)
{
  static const char* javascript =
    "function newGroup(group) {\n"
    "  var groups = db['groups'];\n"
    "  while (1) {\n"
    "    var cursor = groups.find({}, {gid : 1}).sort({gid : -1}).limit(1);\n"
    "    group.gid = cursor.hasNext() ? cursor.next().gid + 1 : 0;\n"
    "    groups.insert(group);\n"
    "    var err = db.getLastErrorObj();\n"
    "    if (err &&  err.code == 11000) {\n"
    "      if (groups.findOne({gid : group.gid}))\n"
    "        continue;\n"
    "      else\n"
    "        return -1;\n"
    "    }\n"
    "    break;\n"
    "  }\n"
    "  return group.gid\n"
    "}";
    
  auto args = db::bson::Group::Serialize(group);
  mongo::BSONElement ret;
  boost::unique_future<bool> future;
  
  auto task = std::make_shared<db::Eval>(javascript, args, ret, future);
  Pool::Queue(task);
  if (!future.get()) throw DBError("Error while creating group.");

  group.gid = static_cast<acl::GroupID>(ret.Double());
  return group.gid != -1;
}

void Save(const acl::Group& group)
{
  mongo::BSONObj obj = db::bson::Group::Serialize(group);
  mongo::Query query = QUERY("gid" << group.GID());
  TaskPtr task(new db::Update("groups", query, obj, true));
  Pool::Queue(task);
}

boost::ptr_vector<acl::Group> 
GetAllPtr(const boost::optional<boost::posix_time::ptime>& modified)
{
  boost::ptr_vector<acl::Group> groups;

  QueryResults results;
  mongo::Query query;
  if (modified) query = QUERY("modified" << BSON("$gte" << db::bson::ToDateT(*modified)));
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groups", query, results, future));
  Pool::Queue(task);

  future.wait();

  for (auto& obj: results)
    groups.push_back(bson::Group::UnserializePtr(obj).release());

  return groups;
}

// need to refactor this so that they both have the same query logic in another function
std::vector<acl::Group> GetAll()
{
  std::vector<acl::Group> groups;

  QueryResults results;
  mongo::Query query;
  boost::unique_future<bool> future;
  TaskPtr task(new db::Select("groups", query, results, future));
  Pool::Queue(task);
  future.wait();

  for (auto& obj: results)
    groups.emplace_back(bson::Group::Unserialize(obj));

  return groups;
}

void Delete(acl::GroupID gid)
{
  mongo::Query query = QUERY("gid" << gid);
  std::vector<TaskPtr> tasks;
  tasks.emplace_back(new db::Delete("groups", query));
  // add groupprofile here later

  for (auto& task: tasks) Pool::Queue(task);
}

}
}

