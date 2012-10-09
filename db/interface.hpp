#ifndef __DB_INTERFACE_HPP
#define __DB_INTERFACE_HPP
#include <mongo/client/dbclient.h>
#include <vector>
#include "acl/types.hpp"
#include "acl/user.hpp"
namespace db 
{
  // initalize
  bool Initalize(); // should only be called once by initial thread
  
  // user functions
  acl::UserID GetNewUserID();
  void SaveUser(const acl::User& user);    
// end
}
#endif
