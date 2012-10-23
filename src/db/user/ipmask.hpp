#ifndef __DB_USER_IPMASK_HPP
#define __DB_USER_IPMASK_HPP

namespace db { namespace ipmask
{

  void Add(const acl::User& user, const std::string& mask);
  void Delete(const acl::User& user, const std::string& mask);
  void GetAll(acl::UserIPMaskMap& userIPMaskMap);

// end
}
}
#endif
