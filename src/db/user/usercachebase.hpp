#ifndef __DB_USERCACHEBASE_HPP
#define __DB_USERCACHEBASE_HPP

#include <string>
#include "acl/types.hpp"

namespace db
{

struct UserCacheBase
{
  virtual ~UserCacheBase() { }
  virtual std::string UIDToName(acl::UserID uid) = 0;
  virtual acl::UserID NameToUID(const std::string& name) = 0;
  virtual acl::GroupID UIDToPrimaryGID(acl::UserID uid) = 0;
  virtual bool IdentIPAllowed(const std::string& identAddress) = 0;  
  virtual bool IdentIPAllowed(const std::string& identAddress, acl::UserID uid) = 0;  
};

} /* db namespace */

#endif
