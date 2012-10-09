#ifndef __DB_INTERFACE_HPP
#define __DB_INTERFACE_HPP
#include <mongo/client/dbclient.h>
#include <vector>
#include "acl/types.hpp"
namespace db 
{
  acl::UserID GetNewUserID();
  void SaveUser(const acl::User& user);    
// end
}
#endif
