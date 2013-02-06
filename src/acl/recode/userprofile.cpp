#include <cassert>
#include "acl/recode/userprofile.hpp"
#include "util/passwd.hpp"
#include "acl/recode/userprofiledb.hpp"

namespace acl { namespace recode
{

UserProfile::UserProfile() :
  db(*this),
  primaryGid(-1),
  creator(-1),
  created(boost::gregorian::day_clock::local_day()),
  weeklyAllotment(0),
  homeDir("/"),
  idleTime(-1),
  numLogins(1),
  maxDownSpeed(0),
  maxUpSpeed(0),
  maxSimDown(-1),
  maxSimUp(-1),
  loggedIn(0)
{
  ratio.insert(std::make_pair("", 3));
}

UserProfile::UserProfile(const User& user) :
  User(user),
  db(*this),
  primaryGid(-1),
  creator(-1),
  created(boost::gregorian::day_clock::local_day()),
  weeklyAllotment(0),
  homeDir("/"),
  idleTime(-1),
  numLogins(1),
  maxDownSpeed(0),
  maxUpSpeed(0),
  maxSimDown(-1),
  maxSimUp(-1),
  loggedIn(0)
{
  ratio.insert(std::make_pair("", 3));
}

UserProfile::~UserProfile()
{
}

bool UserProfile::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  return HexEncode(HashPassword(password, HexDecode(salt))) == this->password;
}

void UserProfile::SetPassword(const std::string& password)
{
  using namespace util::passwd;  
  std::string rawSalt = GenerateSalt();
  this->password = HexEncode(HashPassword(password, rawSalt));
  salt = HexEncode(rawSalt);
  db->SavePassword();
}

void UserProfile::SetFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  this->flags = flags;
  db->SaveFlags();
}

void UserProfile::AddFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  for (char ch: flags)
  {
    if (this->flags.find(ch) == std::string::npos) this->flags += ch;
  }
  
  std::sort(this->flags.begin(), this->flags.end());
  db->SaveFlags();
}

void UserProfile::AddFlag(Flag flag)
{
  AddFlags(std::string(1, static_cast<char>(flag)));
}

void UserProfile::DelFlags(const std::string& flags)
{
  for (char ch: flags)
  {
    std::string::size_type pos = this->flags.find(ch);
    if (pos != std::string::npos) this->flags.erase(pos, 1);
  }
  db->SaveFlags();
}

void UserProfile::DelFlag(Flag flag)
{
  DelFlags(std::string(1, static_cast<char>(flag)));
}

bool UserProfile::CheckFlags(const std::string& flags) const
{
  for (char ch: flags)
  {
    if (this->flags.find(ch) != std::string::npos) return true;
  }
  return false;
}

bool UserProfile::CheckFlag(Flag flag) const
{
  return this->flags.find(static_cast<char>(flag)) != std::string::npos;
}

bool UserProfile::HasSecondaryGID(GroupID gid) const
{
  return std::find(secondaryGids.begin(), secondaryGids.end(), gid) != secondaryGids.end();
}

bool UserProfile::HasGID(GroupID gid) const
{
  return primaryGid == gid || HasSecondaryGID(gid);
}

void UserProfile::SetPrimaryGID(acl::GroupID gid)
{
  this->primaryGid = gid;
  db->SavePrimaryGID();
}

void UserProfile::AddSecondaryGID(GroupID gid)
{
  secondaryGids.emplace_back(gid);
  db->SaveSecondaryGIDs();
}

void UserProfile::DelSecondaryGID(GroupID gid)
{
  secondaryGids.erase(std::remove(secondaryGids.begin(), secondaryGids.end(), gid), 
      secondaryGids.end());
  db->SaveSecondaryGIDs();
}

void UserProfile::ResetSecondaryGIDs()
{
  secondaryGids.clear();
  db->SaveSecondaryGIDs();
}

bool UserProfile::HasGadminGID(GroupID gid) const
{
  return std::find(gadminGids.begin(), gadminGids.end(), gid) != gadminGids.end();
}

void UserProfile::AddGadminGID(GroupID gid)
{
  gadminGids.insert(gid);
  db->SaveGadminGIDs();
  if (!CheckFlag(Flag::Gadmin)) AddFlag(Flag::Gadmin);
}

void UserProfile::DelGadminGID(GroupID gid)
{
  gadminGids.erase(gid);
  db->SaveGadminGIDs();
  if (gadminGids.empty()) DelFlag(Flag::Gadmin);
}

void UserProfile::SetWeeklyAllotment(long long weeklyAllotment)
{
  this->weeklyAllotment = weeklyAllotment;
  db->SaveWeeklyAllotment();
}

void UserProfile::SetHomeDir(const std::string& homeDir)
{
  this->homeDir = homeDir;
  db->SaveHomeDir();
}

void UserProfile::SetIdleTime(int idleTime)
{
  this->idleTime = idleTime;
  db->SaveIdleTime();
}

bool UserProfile::Expired() const
{
  if (!expires) return false;
  return boost::gregorian::day_clock::local_day() >= *expires;
}

void UserProfile::SetExpires(const boost::optional<boost::gregorian::date>& expires)
{
  this->expires = expires;
  db->SaveExpires();
}

void UserProfile::SetNumLogins(int numLogins)
{
  this->numLogins = numLogins;
  db->SaveNumLogins();
}

void UserProfile::Comment(const std::string& comment)
{
  this->comment = comment;
  db->SaveComment();
}

void UserProfile::Tagline(const std::string& tagline)
{
  this->tagline = tagline;
  db->SaveTagline();
}

void UserProfile::SetMaxDownSpeed(long long maxDownSpeed)
{
  this->maxDownSpeed = maxDownSpeed;
  db->SaveMaxDownSpeed();
}

void UserProfile::SetMaxUpSpeed(long long maxUpSpeed)
{
  this->maxUpSpeed = maxUpSpeed;
  db->SaveMaxUpSpeed();
}

void UserProfile::SetMaxSimDown(int maxSimDown)
{
  this->maxSimDown = maxSimDown;
  db->SaveMaxSimDown();
}

void UserProfile::SetMaxSimUp(int maxSimUp)
{
  this->maxSimUp = maxSimUp;
  db->SaveMaxSimUp();
}

void UserProfile::SetLoggedIn()
{
  ++loggedIn;
  lastLogin = boost::posix_time::microsec_clock::local_time();
  db->SaveLoggedIn();
}

int UserProfile::SectionRatio(const std::string& section) const
{
  auto it = ratio.find(section);
  return it != ratio.end() ? it->second : -1;
}

void UserProfile::SetSectionRatio(const std::string& section, int ratio)
{
  this->ratio[section] = ratio;
  db->SaveRatio();
}

long long UserProfile::SectionCredits(const std::string& section) const
{
  auto it = credits.find(section);
  return it != credits.end() ? it->second : -1;
}

void UserProfile::IncrSectionCredits(const std::string& section, long long kBytes)
{
  long long newCredits;
  db->IncrCredits(section, kBytes, newCredits);
  this->credits[section] = newCredits;
}

bool UserProfile::DecrSectionCredits(const std::string& section, long long kBytes)
{
  long long newCredits;
  if (!db->DecrCredits(section, kBytes, false, newCredits)) return false;
  credits[section] = newCredits;
  return true;
}

void UserProfile::DecrSectionCreditsForce(const std::string& section, long long kBytes)
{
  long long newCredits;
  (void) db->DecrCredits(section, kBytes, false, newCredits);
  credits[section] = newCredits;
}

} /* recode namespace */
} /* acl namespace */
