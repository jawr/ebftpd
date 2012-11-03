#ifndef __DB_USER_USERPROFILE_HPP
#define __DB_USER_USERPROFILE_HPP

#include <vector>
#include <map>
#include <boost/ptr_container/ptr_vector.hpp>
#include <mongo/client/dbclient.h>
#include "acl/userprofile.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"
#include "acl/user.hpp"

namespace db { namespace userprofile
{

  void Save(const acl::UserProfile& profile);
  void GetAll(std::vector<acl::UserProfile>& profiles);
  acl::UserProfile Get(const acl::UserID& uid);
  void GetSelection(boost::ptr_vector<acl::User>& users,
    std::map<acl::UserID, acl::UserProfile>& profiles);

  // setters
  void Set(const acl::UserID uid, mongo::BSONObj obj);

  util::Error SetRatio(const acl::UserID uid, const std::string& value);
  util::Error SetWeeklyAllotment(const acl::UserID uid, const std::string& value);
  util::Error SetHomeDir(const acl::UserID uid, const std::string& value);
  util::Error SetStartupDir(const acl::UserID uid, const std::string& value);
  util::Error SetIdleTime(const acl::UserID uid, const std::string& value);
  util::Error SetExpires(const acl::UserID uid, const std::string& value);
  util::Error SetNumLogins(const acl::UserID uid, const std::string& value);
  util::Error SetTagline(const acl::UserID uid, const std::string& value);
  util::Error SetComment(const acl::UserID uid, const std::string& value);
  util::Error SetMaxDlSpeed(const acl::UserID uid, const std::string& value);
  util::Error SetMaxUlSpeed(const acl::UserID uid, const std::string& value);
  util::Error SetMaxSimDl(const acl::UserID uid, const std::string& value);
  util::Error SetMaxSimUl(const acl::UserID uid, const std::string& value);

// end
}
}
#endif
