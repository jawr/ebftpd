#ifndef __ACL_GROUP_HPP
#define __ACL_GROUP_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"

namespace db { namespace bson
{
struct Group;
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
  Group() : modified(boost::posix_time::microsec_clock::local_time()), gid(-1) {}
  Group(const std::string& name, GroupID gid) : 
    modified(boost::posix_time::microsec_clock::local_time()), 
    name(name), gid(gid)
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
};

} /* acl namespace */

#endif
