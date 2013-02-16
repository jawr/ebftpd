#include <cassert>
#include "acl/user.hpp"
#include "util/passwd.hpp"
#include "db/user.hpp"
#include "util/error.hpp"

namespace acl
{

User::User() :
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

void User::Rename(const std::string& name)
{
  this->name = name; 
  db->SaveName();
}

void User::AddIPMask(const std::string& ipMask)
{
  ipMasks.push_back(ipMask);
  db->SaveIPMasks();
}

void User::DelIPMask(const std::string& ipMask)
{
  auto it = std::find(ipMasks.begin(), ipMasks.end(), ipMask);
  if (it != ipMasks.end())
  {
    ipMasks.erase(it);
    db->SaveIPMasks();
  }
}

bool User::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  return HexEncode(HashPassword(password, HexDecode(salt))) == this->password;
}

void User::SetPassword(const std::string& password)
{
  using namespace util::passwd;  
  std::string rawSalt = GenerateSalt();
  this->password = HexEncode(HashPassword(password, rawSalt));
  salt = HexEncode(rawSalt);
  db->SavePassword();
}

void User::SetFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  this->flags = flags;
  db->SaveFlags();
}

void User::AddFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  for (char ch: flags)
  {
    if (this->flags.find(ch) == std::string::npos) this->flags += ch;
  }
  
  std::sort(this->flags.begin(), this->flags.end());
  db->SaveFlags();
}

void User::AddFlag(Flag flag)
{
  AddFlags(std::string(1, static_cast<char>(flag)));
}

void User::DelFlags(const std::string& flags)
{
  for (char ch: flags)
  {
    std::string::size_type pos = this->flags.find(ch);
    if (pos != std::string::npos) this->flags.erase(pos, 1);
  }
  db->SaveFlags();
}

void User::DelFlag(Flag flag)
{
  DelFlags(std::string(1, static_cast<char>(flag)));
}

bool User::CheckFlags(const std::string& flags) const
{
  for (char ch: flags)
  {
    if (this->flags.find(ch) != std::string::npos) return true;
  }
  return false;
}

bool User::CheckFlag(Flag flag) const
{
  return this->flags.find(static_cast<char>(flag)) != std::string::npos;
}

bool User::HasSecondaryGID(GroupID gid) const
{
  return std::find(secondaryGids.begin(), secondaryGids.end(), gid) != secondaryGids.end();
}

bool User::HasGID(GroupID gid) const
{
  return primaryGid == gid || HasSecondaryGID(gid);
}

void User::SetPrimaryGID(acl::GroupID gid)
{
  this->primaryGid = gid;
  db->SavePrimaryGID();
}

void User::AddSecondaryGID(GroupID gid)
{
  secondaryGids.emplace_back(gid);
  db->SaveSecondaryGIDs();
}

void User::DelSecondaryGID(GroupID gid)
{
  secondaryGids.erase(std::remove(secondaryGids.begin(), secondaryGids.end(), gid), 
      secondaryGids.end());
  db->SaveSecondaryGIDs();
}

void User::ResetSecondaryGIDs()
{
  secondaryGids.clear();
  db->SaveSecondaryGIDs();
}

bool User::HasGadminGID(GroupID gid) const
{
  return std::find(gadminGids.begin(), gadminGids.end(), gid) != gadminGids.end();
}

void User::AddGadminGID(GroupID gid)
{
  gadminGids.insert(gid);
  db->SaveGadminGIDs();
  if (!CheckFlag(Flag::Gadmin)) AddFlag(Flag::Gadmin);
}

void User::DelGadminGID(GroupID gid)
{
  gadminGids.erase(gid);
  db->SaveGadminGIDs();
  if (gadminGids.empty()) DelFlag(Flag::Gadmin);
}

void User::SetWeeklyAllotment(long long weeklyAllotment)
{
  this->weeklyAllotment = weeklyAllotment;
  db->SaveWeeklyAllotment();
}

void User::SetHomeDir(const std::string& homeDir)
{
  this->homeDir = homeDir;
  db->SaveHomeDir();
}

void User::SetIdleTime(int idleTime)
{
  this->idleTime = idleTime;
  db->SaveIdleTime();
}

bool User::Expired() const
{
  if (!expires) return false;
  return boost::gregorian::day_clock::local_day() >= *expires;
}

void User::SetExpires(const boost::optional<boost::gregorian::date>& expires)
{
  this->expires = expires;
  db->SaveExpires();
}

void User::SetNumLogins(int numLogins)
{
  this->numLogins = numLogins;
  db->SaveNumLogins();
}

void User::Comment(const std::string& comment)
{
  this->comment = comment;
  db->SaveComment();
}

void User::Tagline(const std::string& tagline)
{
  this->tagline = tagline;
  db->SaveTagline();
}

void User::SetMaxDownSpeed(long long maxDownSpeed)
{
  this->maxDownSpeed = maxDownSpeed;
  db->SaveMaxDownSpeed();
}

void User::SetMaxUpSpeed(long long maxUpSpeed)
{
  this->maxUpSpeed = maxUpSpeed;
  db->SaveMaxUpSpeed();
}

void User::SetMaxSimDown(int maxSimDown)
{
  this->maxSimDown = maxSimDown;
  db->SaveMaxSimDown();
}

void User::SetMaxSimUp(int maxSimUp)
{
  this->maxSimUp = maxSimUp;
  db->SaveMaxSimUp();
}

void User::SetLoggedIn()
{
  ++loggedIn;
  db->SaveLoggedIn();

  lastLogin = boost::posix_time::microsec_clock::local_time();
  db->SaveLastLogin();
}

int User::SectionRatio(const std::string& section) const
{
  auto it = ratio.find(section);
  return it != ratio.end() ? it->second : -1;
}

void User::SetSectionRatio(const std::string& section, int ratio)
{
  this->ratio[section] = ratio;
  db->SaveRatio();
}

long long User::SectionCredits(const std::string& section) const
{
  auto it = credits.find(section);
  return it != credits.end() ? it->second : -1;
}

void User::IncrSectionCredits(const std::string& section, long long kBytes)
{
  long long newCredits;
  db->IncrCredits(section, kBytes, newCredits);
  this->credits[section] = newCredits;
}

bool User::DecrSectionCredits(const std::string& section, long long kBytes)
{
  long long newCredits;
  if (!db->DecrCredits(section, kBytes, false, newCredits)) return false;
  credits[section] = newCredits;
  return true;
}

void User::DecrSectionCreditsForce(const std::string& section, long long kBytes)
{
  long long newCredits;
  (void) db->DecrCredits(section, kBytes, false, newCredits);
  credits[section] = newCredits;
}

boost::optional<User> User::Load(acl::UserID uid)
{
  return db::User::Load(uid);
}

User User::Skeleton(acl::UserID uid)
{
  User user;
  user.id = uid;
  return user;
}

User User::Create(const std::string& name, const std::string& password, 
                  acl::UserID creator)
{
  try
  {
    User user;
    user.name = name;
    user.SetPassword(password);
    user.creator = creator;
    user.id = user.db->Create();
    return user;
  }
  catch (const db::DBKeyError&)
  {
    throw util::RuntimeError("User already exists");
  }
}

User User::FromTemplate(const std::string& name, const std::string& password,
                        acl::UserID creator, const User& templateUser)
{
  try
  {
    User user(templateUser);
    user.name = name;
    user.SetPassword(password);
    user.creator = creator;
    user.DelFlag(Flag::Template);
    user.id = user.db->Create();
    return user;
  }
  catch (const db::DBKeyError&)
  {
    throw util::RuntimeError("User already exists");
  }
}

std::string UIDToName(acl::UserID uid)
{
  return db::UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  return db::NameToUID(name);
}

} /* acl namespace */
