#include <fnmatch.h>
#include "acl/repository.hpp"

namespace acl
{

void PathRepository::Insert(const std::string& pathMask, const ACL& acl)
{
  acls.push_back(std::make_pair(pathMask, acl));
}

bool PathRepository::Lookup(const std::string& path, ACL& acl)
{
  for (Container::const_iterator it = acls.begin();
       it != acls.end(); ++it)
  {
    if (!fnmatch(it->first.c_str(), path.c_str(), 0))
    {
      acl = it->second;
      return true;
    }
  }
  return false;
}

void GeneralRepository::Insert(const std::string& key, const ACL& acl)
{
  acls.insert(std::make_pair(key, acl));
}

bool GeneralRepository::Lookup(const std::string& key, ACL& acl)
{
  Container::const_iterator it = acls.find(key);
  if (it == acls.end()) return false;
  acl = it->second;
  return true;
}

} /* acl namespace */
