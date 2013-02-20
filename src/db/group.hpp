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
struct GroupData;
} 

namespace db
{

class Group
{
  acl::GroupData& group;
  
  void SaveField(const std::string& field);
  
public:
  Group(acl::GroupData& group) :  group(group) { }
  
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
  
  static boost::optional<acl::GroupData> Load(acl::GroupID gid);
  static boost::optional<acl::GroupData> Load(const std::string& name);
};


std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
std::vector<acl::GroupData> GetGroups(const std::string& multiStr = "*");


} /* db namespace */

#endif
