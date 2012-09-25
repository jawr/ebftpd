#ifndef __ACL_REPOSITORY_HPP
#define __ACL_REPOSITORY_HPP

#include <string>
#include <vector>
#include <utility>
#include <boost/unordered_map.hpp>
#include "acl/acl.hpp"

namespace acl
{

class Repository
{
public:
  virtual ~Repository() { }
  virtual void Insert(const std::string& id, const ACL& acl) = 0;
  virtual bool Lookup(const std::string& id, ACL& acl) = 0;
};

class PathRepository : public Repository
{
  typedef std::vector<std::pair<std::string, ACL> > Container;
  Container acls;
  
public:
  void Insert(const std::string& pathMask, const ACL& acl);
  bool Lookup(const std::string& path, ACL& acl);
};

class GeneralRepository : public Repository
{
  typedef boost::unordered_map<std::string, ACL> Container;
  Container acls;
  
public:
  void Insert(const std::string& key, const ACL& acl);
  bool Lookup(const std::string& key, ACL& acl);
};

} /* acl namespace */

#endif
