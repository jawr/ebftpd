#ifndef __DB_GROUP_HPP
#define __DB_GROUP_HPP

#include <string>
#include <memory>
#include <boost/optional/optional_fwd.hpp>
#include "acl/types.hpp"

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
  
  void UpdateLog() const;
  void SaveField(const std::string& field);
  
public:
  Group(acl::GroupData& group) :  group(group) { }
  
  bool Create();
  bool SaveName();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
  
  int NumSlotsUsed() const;
  int NumMembers() const;
  int NumLeeches() const;
  int NumAllotments() const;
  long long TotalAllotmentSize() const;

  void Purge() const;
  
  static boost::optional<acl::GroupData> Load(acl::GroupID gid);
  static boost::optional<acl::GroupData> Load(const std::string& name);
};


std::vector<acl::GroupID> GetGIDs(const std::string& multiStr = "*");
std::vector<acl::GroupData> GetGroups(const std::string& multiStr = "*");


} /* db namespace */

#endif
