#ifndef __ACL_IPMASKCACHE_HPP
#define __ACL_IPMASKCACHE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <boost/thread/shared_mutex.hpp>
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"
#include "acl/replicable.hpp"

namespace acl
{

typedef std::unordered_map<acl::UserID, std::vector<std::string>> UserIPMaskMap;

class IpMaskCache : public Replicable
{
  boost::shared_mutex mtx;
  std::unique_ptr<UserIPMaskMap> userIPMaskMap;
  
  unsigned changes;

  static IpMaskCache instance;
  
  IpMaskCache() : changes(0) { }
  
  void SaveAdd(acl::UserID uid, const std::string& mask);
  void SaveDelete(acl::UserID uid, const std::string& mask);

public:
  bool Replicate();

  static void Initialize();
  static bool Check(const std::string& addr);
  static bool Check(acl::UserID uid, const std::string& addr);
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
