#ifndef __DB_GROUPPROFILEDB_HPP
#define __DB_GROUPPROFILEDB_HPP

#include <iostream>

namespace mongo
{
class BSONObj;
class Query;
}

namespace acl { namespace recode
{
class GroupProfile;
} 
} 

namespace db { namespace recode
{

class GroupProfileDB
{
  acl::recode::GroupProfile& profile;
  
  mongo::BSONObj Serialize();
  bool Unserialize(const mongo::BSONObj& obj);
  mongo::Query DefaultQuery();
  void SaveField(const std::string& field);
  
public:
  GroupProfileDB(acl::recode::GroupProfile& profile) :  profile(profile) { }
  
  bool Load();
  void Save();
  void SaveDescription();
  void SaveComment();
  void SaveSlots();
  void SaveLeechSlots();
  void SaveAllotmentSlots();
  void SaveMaxAllotmentSize();
  void SaveMaxLogins();
};

} /* recode namespace */
} /* db namespace */

#endif
