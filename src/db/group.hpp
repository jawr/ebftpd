#ifndef __DB_GROUP_HPP
#define __DB_GROUP_HPP

#include <string>
#include <memory>
#include "acl/types.hpp"
#include "db/serialization.hpp"

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
  bool SaveName();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
  
  long long NumMembers() const;
  void Purge() const;
  
  static boost::optional<acl::Group> Load(acl::GroupID gid);
};


//template <> mongo::BSONObj Serialize<acl::Group>(const acl::Group& group);
//template <> acl::Group Unserialize<acl::Group>(const mongo::BSONObj& obj);

std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
std::vector<acl::Group> GetGroups(const std::string& multiStr = "*");


} /* db namespace */

#endif
