#ifndef __DB_GROUPDB_HPP
#define __DB_GROUPDB_HPP

#include <iostream>

namespace mongo
{
class BSONObj;
class Query;
}

namespace acl { namespace recode
{
class Group;
} 
} 

namespace db { namespace recode
{

class GroupDB
{
  acl::recode::Group& group;
  
  bool Unserialize(const mongo::BSONObj& obj);
  mongo::Query DefaultQuery();
  
public:
  GroupDB(acl::recode::Group& group) : group(group) { }

  bool Load();
  void SaveName();
};

} /* recode namespace */
} /* db namespace */

#endif
