#ifndef __FS_OWNER_HPP
#define __FS_OWNER_HPP

#include "acl/types.hpp"
#include "fs/path.hpp"

namespace util
{
class Error;
}

namespace fs
{

class Owner
{
  acl::UserID uid;
  acl::GroupID gid;
  
public:
  Owner(acl::UserID uid, acl::GroupID gid) : uid(uid), gid(gid) { }
  
  acl::UserID UID() const { return uid; }
  acl::GroupID GID() const { return gid; }
};

Owner GetOwner(const RealPath& path);
util::Error SetOwner(const RealPath& path, const Owner& owner);

std::ostream& operator<<(std::ostream& os, const Owner& owner);

} /* fs namespace */



#endif
