#ifndef __ACL_USER_HPP
#define __ACL_USER_HPP

#include <functional>
#include <vector>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "acl/types.hpp"
#include "util/keybase.hpp"
#include "acl/recode/dbproxy.hpp"
#include "acl/recode/userdb.hpp"

namespace acl { namespace recode
{

class UserProfile;
class User;

typedef DBProxy<User, acl::UserID, db::recode::UserDB> UserDBProxy;

class User
{
  UserDBProxy db;
  
protected:
  boost::posix_time::ptime modified;
  acl::UserID id;
  std::string name;
  std::vector<std::string> ipMasks;
  
  User(const UserProfile&) = delete;
  User& operator=(const UserProfile&) = delete;
  
public:
  class SetIDKey : util::KeyBase {  SetIDKey() { } };

  User();
  User(const std::string& name);
  
  virtual ~User();
  
  acl::UserID ID() const { return id; }
  void SetID(acl::UserID id, const SetIDKey& key) { this->id = id; }
  
  const std::string& Name() const { return name; }
  void Rename(const std::string& name);

  const std::vector<std::string>& IPMasks() const { return ipMasks; }
  void AddIPMask(const std::string& ipMask);
  void DelIPMask(const std::string& ipMask);
  
  friend class DBProxy<User, acl::UserID, db::recode::UserDB>;
};

void UserUpdatedSlot(const std::function<void(acl::UserID)>& slot);

} /* recode namespace */
} /* acl namespace */

#endif
