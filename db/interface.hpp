#ifndef __DB_INTERFACE_HPP
#define __DB_INTERFACE_HPP
#include <mongo/client/dbclient.h>
#include <vector>
#include <memory>
#include "acl/types.hpp"
#include "acl/user.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
namespace db 
{
  // initalize
  bool Initalize(); // should only be called once by initial thread
  
  // user functions
  acl::UserID GetNewUserID();
  void SaveUser(const acl::User& user);    
  void GetUsers(std::vector<acl::User*>& users);
// end
}
#endif
