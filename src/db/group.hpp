#ifndef __DB_GROUP_HPP
#define __DB_GROUP_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
class Query;
}

namespace acl
{
class Group;
} 

namespace db
{

class Group
{
  acl::Group& group;
  
  void SaveField(const std::string& field);
  
public:
  Group(acl::Group& group) :  group(group) { }
  
  acl::GroupID Create();
  void SaveName();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
  
  static boost::optional<acl::Group> Load(acl::GroupID gid);
};


mongo::BSONObj Serialize(const acl::Group& group);
acl::Group Unserialize(const mongo::BSONObj& obj);

struct GroupCache
{
  virtual std::string GIDToName(acl::GroupID gid) = 0;
  virtual acl::GroupID NameToGID(const std::string& name) = 0;
};

void RegisterGroupCache(const std::shared_ptr<GroupCache>& cache);

std::string GIDToName(acl::GroupID gid);
acl::GroupID NameToGID(const std::string& name);

} /* db namespace */

#endif
