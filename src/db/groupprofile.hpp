#ifndef __DB_GROUPPROFILE_HPP
#define __DB_GROUPPROFILE_HPP

#include <iostream>
#include "acl/types.hpp"

namespace mongo
{
class BSONObj;
class Query;
}

namespace acl
{
class GroupProfile;
} 

namespace db
{

class GroupProfile
{
  acl::GroupProfile& profile;
  
  void SaveField(const std::string& field);
  
public:
  GroupProfile(acl::GroupProfile& profile) :  profile(profile) { }
  
  void Save();
  void SaveName();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
  
  static boost::optional<acl::GroupProfile> Load(acl::GroupID gid);
};


mongo::BSONObj Serialize(const acl::GroupProfile& profile);
acl::GroupProfile Unserialize(const mongo::BSONObj& obj);

} /* db namespace */

#endif
