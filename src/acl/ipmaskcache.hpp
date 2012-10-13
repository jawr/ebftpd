#ifndef __ACL_IpMaskCACHE_HPP
#define __ACL_IpMaskCACHE_HPP

#include <string>
#include <vector>
#include <boost/thread/shared_mutex.hpp>
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"

namespace acl
{
class IpMaskCache
{
  boost::shared_mutex mtx;

  UserMaskMap userMaskMap;

  static IpMaskCache instance;
  IpMaskCache() : mtx() {};

public:
  
  static void Initalize();
  static bool Check(const std::string& addr);
  static util::Error Add(const acl::User& user, const std::string& mask,
    std::vector<std::string>& deleted);
  static util::Error Delete(const acl::User& user, const std::string& mask);

  static util::Error List(const acl::User& user, 
    std::vector<std::string>& masks);
  
};


// end
}
#endif
