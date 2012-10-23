#ifndef __DB_USER_USER_HPP
#define __DB_USER_USER_HPP

#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "acl/ipmaskcache.hpp"


namespace db { namespace user
{
  acl::UserID GetNewUserID();
  void Save(const acl::User& user);    
  void Delete(const acl::UserID& uid);
  void GetAll(boost::ptr_vector<acl::User>& users);
  void Login(const acl::UserID& user);



// end
}
}
#endif
