#ifndef __DB_USER_USERPROFILE_HPP
#define __DB_USER_USERPROFILE_HPP

#include <vector>
#include <unordered_map>
#include <boost/ptr_container/ptr_vector.hpp>
#include "acl/types.hpp"

namespace acl
{
class UserProfile;
class User;
}

namespace util
{
class Error;
}

namespace db { namespace userprofile
{

void Save(const acl::UserProfile& profile);
void Delete(acl::UserID uid);
std::vector<acl::UserProfile> GetAll();
acl::UserProfile Get(acl::UserID uid);

std::unordered_map<acl::UserID, acl::UserProfile>
GetSelection(const std::vector<acl::User>& users);

bool DecrCredits(acl::UserID uid, long long kBytes, 
        const std::string& section, bool negativeOkay);
void IncrCredits(acl::UserID uid, long long kBytes,
        const std::string& section);

void SetWeeklyAllotment(acl::UserID uid, int allotment);
void SetHomeDir(acl::UserID uid, const std::string& path);
void SetIdleTime(acl::UserID uid, int idleTime);
void SetExpires(acl::UserID uid, const boost::optional<boost::gregorian::date>& date);
void SetNumLogins(acl::UserID uid, int logins);
void SetComment(acl::UserID uid, const std::string& comment);
void SetMaxDownSpeed(acl::UserID uid, int speed);
void SetMaxUpSpeed(acl::UserID uid, int speed);
void SetMaxSimDown(acl::UserID uid, int logins);
void SetMaxSimUp(acl::UserID uid, int logins);
void SetRatio(acl::UserID uid, const std::string& section, int ratio);
void SetTagline(acl::UserID uid, const std::string& tagline);

void Login(acl::UserID uid);
// end
}
}
#endif
