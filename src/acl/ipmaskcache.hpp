#ifndef __ACL_IPMASKCACHE_HPP
#define __ACL_IPMASKCACHE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"

namespace acl
{

typedef std::unordered_map<acl::UserID, std::vector<std::string>> UserIPMaskMap;

class IpMaskCache
{
  boost::shared_mutex mtx;
  UserIPMaskMap userIPMaskMap;

  static IpMaskCache instance;
  IpMaskCache() {}

public:
  
  static void Initialize();
  static bool Check(const std::string& addr);
  static util::Error Add(acl::UserID uid, const std::string& mask,
      std::vector<std::string>& deleted);
  static util::Error Add(acl::UserID uid, const std::string& mask)
  {
    std::vector<std::string> deleted;
    return Add(uid, mask, deleted);
  }
  static util::Error Delete(acl::UserID uid, int idx, std::string& deleted);
  static util::Error DeleteAll(acl::UserID uid, std::vector<std::string>& deleted);
  
  static util::Error List(acl::UserID uid, 
      std::vector<std::string>& masks);  
};


// end
}
#endif
