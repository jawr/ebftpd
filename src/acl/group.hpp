#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"

namespace acl
{
class Group;
}

namespace db { namespace bson
{
struct Group;
}

namespace group
{
bool Create(acl::Group& group);
}
}

namespace acl
{

class Group
{
  boost::posix_time::ptime modified;
  std::string name;
  GroupID gid;
  
public:
  Group() : modified(boost::posix_time::microsec_clock::local_time()), gid(-1) { }
  Group(const std::string& name) : 
    modified(boost::posix_time::microsec_clock::local_time()), 
    name(name), gid(-1)
  { }
  
  const boost::posix_time::ptime& Modified() const { return modified; }
  
  const std::string& Name() const { return name; }
  void SetName(const std::string& name)
  {
    modified = boost::posix_time::microsec_clock::local_time();
    this->name = name;
  }
  
  GroupID GID() const { return gid; }
  
  friend struct db::bson::Group;
  friend bool db::group::Create(acl::Group& group);
};

} /* acl namespace */

#endif
