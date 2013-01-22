#ifndef __DB_USER_USERPROFILE_HPP
#define __DB_USER_USERPROFILE_HPP

#include <vector>
#include <unordered_map>
#include <boost/ptr_container/ptr_vector.hpp>
#include <mongo/client/dbclient.h>
#include "acl/userprofile.hpp"
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "util/error.hpp"
#include "acl/user.hpp"

namespace db { namespace userprofile
{

void Save(const acl::UserProfile& profile);
void Delete(acl::UserID uid);
std::vector<acl::UserProfile> GetAll();
acl::UserProfile Get(acl::UserID uid);

std::unordered_map<acl::UserID, acl::UserProfile> GetSelection(
  const std::vector<acl::User>& users);

// setters
void Set(acl::UserID uid, mongo::BSONObj obj);

util::Error SetRatio(acl::UserID uid, const std::string& value);
util::Error SetWeeklyAllotment(acl::UserID uid, const std::string& value);
util::Error SetHomeDir(acl::UserID uid, const std::string& value);
util::Error SetStartupDir(acl::UserID uid, const std::string& value);
util::Error SetIdleTime(acl::UserID uid, const std::string& value);
util::Error SetExpires(acl::UserID uid, std::string& value);
util::Error SetNumLogins(acl::UserID uid, const std::string& value);
util::Error SetComment(acl::UserID uid, const std::string& value);
util::Error SetMaxDlSpeed(acl::UserID uid, const std::string& value);
util::Error SetMaxUlSpeed(acl::UserID uid, const std::string& value);
util::Error SetMaxSimDl(acl::UserID uid, const std::string& value);
util::Error SetMaxSimUl(acl::UserID uid, const std::string& value);
void SetSectionRatio(acl::UserID uid, const std::string& section, int ratio);

void Login(acl::UserID uid);
// end
}
}
#endif
