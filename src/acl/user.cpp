#include <algorithm>
#include <cassert>
#include "acl/user.hpp"
#include "util/passwd.hpp"
#include "db/user.hpp"
#include "util/error.hpp"
#include "db/error.hpp"
#include "util/string.hpp"
#include "util/verify.hpp"

namespace acl
{

UserData::UserData() :
  modified(boost::posix_time::microsec_clock::local_time()),
  id(-1),  
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

User::User() :
  db(data)
{
}

User::User(UserData&& data_) :
  data(data_),
  db(data)
{
}

User& User::operator=(User&& rhs)
{
  data = rhs.data;
  return *this;
}

User& User::operator=(const User& rhs)
{
  data = rhs.data;
  return *this;
}

User::User(User&& other) :
  data(other.data),
  db(data)
{
}

User::User(const User& other) :
  data(other.data),
  db(data)
{
}

User::~User()
{
}

bool User::Rename(const std::string& name)
{
  std::string oldName = this->data.name;
  this->data.name = name; 
  if (!db->SaveName())
  {
    this->data.name.swap(oldName);
    return false;
  }
  return true;
}

bool User::AddIPMask(const std::string& ipMask, std::vector<std::string>* deleted)
{
  if (std::find_if(data.ipMasks.begin(), data.ipMasks.end(),
        [&](const std::string& mask)
        {
          return util::string::WildcardMatch(mask, ipMask, true);
        }) != data.ipMasks.end())
  {
    return false;
  }
  
  data.ipMasks.push_back(ipMask);
 
  for (auto it = data.ipMasks.begin(); it != data.ipMasks.end();)
  {
    if (util::string::WildcardMatch(ipMask, *it, true))
    {
      if (deleted) deleted->emplace_back(*it);
      it = data.ipMasks.erase(it);
    }
    else
      ++it;
  }
  
  db->SaveIPMasks();
  return true;
}

void User::DelIPMask(const std::string& ipMask)
{
  auto it = std::find(data.ipMasks.begin(), data.ipMasks.end(), ipMask);
  if (it != data.ipMasks.end())
  {
    data.ipMasks.erase(it);
    db->SaveIPMasks();
  }
}

void User::DelIPMask(size_t index)
{
  verify(index < data.ipMasks.size());
  data.ipMasks.erase(data.ipMasks.begin() + index);
  db->SaveIPMasks();
}

void User::ClearIPMasks()
{
  data.ipMasks.clear();
  db->SaveIPMasks();
}

bool User::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  return HexEncode(HashPassword(password, HexDecode(data.salt))) == this->data.password;
}

void User::SetPassword(const std::string& password)
{
  using namespace util::passwd;  
  std::string rawSalt = GenerateSalt();
  this->data.password = HexEncode(HashPassword(password, rawSalt));
  data.salt = HexEncode(rawSalt);
  db->SavePassword();
}

void User::SetFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  this->data.flags = flags;
  db->SaveFlags();
}

void User::AddFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  for (char ch: flags)
  {
    if (this->data.flags.find(ch) == std::string::npos) this->data.flags += ch;
  }
  
  std::sort(this->data.flags.begin(), this->data.flags.end());
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
    std::string::size_type pos = this->data.flags.find(ch);
    if (pos != std::string::npos) this->data.flags.erase(pos, 1);
  }
  db->SaveFlags();
}

void User::DelFlag(Flag flag)
{
  DelFlags(std::string(1, static_cast<char>(flag)));
}

bool User::HasFlags(const std::string& flags) const
{
  for (char ch: flags)
  {
    if (this->data.flags.find(ch) != std::string::npos) return true;
  }
  return false;
}

bool User::HasFlag(Flag flag) const
{
  return this->data.flags.find(static_cast<char>(flag)) != std::string::npos;
}

bool User::HasSecondaryGID(GroupID gid) const
{
  return std::find(data.secondaryGids.begin(), data.secondaryGids.end(), gid) != 
        data.secondaryGids.end();
}

bool User::HasGID(GroupID gid) const
{
  return data.primaryGid == gid || HasSecondaryGID(gid);
}

void User::SetPrimaryGID(acl::GroupID gid)
{
  if (data.primaryGid != -1)
  {
    data.secondaryGids.insert(data.secondaryGids.begin(), data.primaryGid);
    db->SaveSecondaryGIDs();
  }
  
  data.primaryGid = gid;
  db->SavePrimaryGID();
}

void User::AddGIDs(const std::vector<acl::GroupID>& gids)
{
  if (gids.empty()) return;
  
  size_t offset = 0;
  if (data.primaryGid == -1)
  {
    data.primaryGid = gids.front();
    ++offset;
    db->SavePrimaryGID();
  }
  
  if (gids.size() > offset)
  {
    std::copy_if(gids.begin() + offset, gids.end(), std::back_inserter(data.secondaryGids),
        [&](acl::GroupID gid)
        {
          return std::find(data.secondaryGids.begin(), data.secondaryGids.end(), gid) == 
                data.secondaryGids.end();
        });
    db->SaveSecondaryGIDs();
  }
}

void User::DelGIDs(const std::vector<acl::GroupID>& gids)
{
  auto preSize = data.secondaryGids.size();
  data.secondaryGids.erase(std::remove_if(data.secondaryGids.begin(), data.secondaryGids.end(),
      [&](acl::GroupID gid)
      {
        return std::find(gids.begin(), gids.end(), gid) != gids.end();
      }), data.secondaryGids.end());
        
  if (std::find(gids.begin(), gids.end(), data.primaryGid) != gids.end())
  {
    if (data.secondaryGids.empty()) data.primaryGid = -1;
    else
    {
      data.primaryGid = data.secondaryGids.front();
      data.secondaryGids.erase(data.secondaryGids.begin());
    }
    
    db->SavePrimaryGID();
  }
  
  if (data.secondaryGids.size() < preSize) db->SaveSecondaryGIDs();
}

void User::SetGIDs(const std::vector<acl::GroupID>& gids)
{
  data.primaryGid = -1;
  data.secondaryGids.clear();
  
  if (!gids.empty())
  {
    data.primaryGid = gids.front();
    data.secondaryGids.assign(gids.begin() + 1, gids.end());
  }
  
  db->SavePrimaryGID();
  db->SaveSecondaryGIDs();
}

void User::ToggleGIDs(const std::vector<acl::GroupID>& gids)
{
  std::vector<acl::GroupID> diffGids;
  
  data.secondaryGids.insert(data.secondaryGids.begin(), data.primaryGid);
  
  std::copy_if(data.secondaryGids.begin(), data.secondaryGids.end(), std::back_inserter(diffGids),
      [&](acl::GroupID gid)
      {
        return std::find(gids.begin(), gids.end(), gid) == gids.end();
      });
      
  std::copy_if(gids.begin(), gids.end(), std::back_inserter(diffGids),
      [&](acl::GroupID gid)
      {
        return std::find(data.secondaryGids.begin(), data.secondaryGids.end(), gid) == data.secondaryGids.end();
      });
  
  if (diffGids.empty()) data.primaryGid = -1;
  else
  {
    data.primaryGid = diffGids.front();
    data.secondaryGids.assign(diffGids.begin() + 1, diffGids.end());
  }
  
  db->SavePrimaryGID();
  db->SaveSecondaryGIDs();
}

bool User::HasGadminGID(GroupID gid) const
{
  return std::find(data.gadminGids.begin(), data.gadminGids.end(), gid) != data.gadminGids.end();
}

void User::AddGadminGID(GroupID gid)
{
  data.gadminGids.insert(gid);
  db->SaveGadminGIDs();
  if (!HasFlag(Flag::Gadmin)) AddFlag(Flag::Gadmin);
}

void User::DelGadminGID(GroupID gid)
{
  data.gadminGids.erase(gid);
  db->SaveGadminGIDs();
  if (data.gadminGids.empty()) DelFlag(Flag::Gadmin);
}

bool User::ToggleGadminGID(GroupID gid)
{
  if (HasGadminGID(gid))
  {
    DelGadminGID(gid);
    return false;
  }
  
  AddGadminGID(gid);
  return true;
}

void User::SetWeeklyAllotment(long long weeklyAllotment)
{
  this->data.weeklyAllotment = weeklyAllotment;
  db->SaveWeeklyAllotment();
}

void User::SetHomeDir(const std::string& homeDir)
{
  this->data.homeDir = homeDir;
  db->SaveHomeDir();
}

void User::SetIdleTime(int idleTime)
{
  this->data.idleTime = idleTime;
  db->SaveIdleTime();
}

bool User::Expired() const
{
  if (!data.expires) return false;
  return boost::gregorian::day_clock::local_day() >= *data.expires;
}

void User::SetExpires(const boost::optional<boost::gregorian::date>& expires)
{
  this->data.expires = expires;
  db->SaveExpires();
}

void User::SetNumLogins(int numLogins)
{
  this->data.numLogins = numLogins;
  db->SaveNumLogins();
}

void User::SetComment(const std::string& comment)
{
  this->data.comment = comment;
  db->SaveComment();
}

void User::SetTagline(const std::string& tagline)
{
  this->data.tagline = tagline;
  db->SaveTagline();
}

void User::SetMaxDownSpeed(long long maxDownSpeed)
{
  this->data.maxDownSpeed = maxDownSpeed;
  db->SaveMaxDownSpeed();
}

void User::SetMaxUpSpeed(long long maxUpSpeed)
{
  this->data.maxUpSpeed = maxUpSpeed;
  db->SaveMaxUpSpeed();
}

void User::SetMaxSimDown(int maxSimDown)
{
  this->data.maxSimDown = maxSimDown;
  db->SaveMaxSimDown();
}

void User::SetMaxSimUp(int maxSimUp)
{
  this->data.maxSimUp = maxSimUp;
  db->SaveMaxSimUp();
}

void User::SetLoggedIn()
{
  ++data.loggedIn;
  db->SaveLoggedIn();

  data.lastLogin = boost::posix_time::microsec_clock::local_time();
  db->SaveLastLogin();
}

int User::SectionRatio(const std::string& section) const
{
  auto it = data.ratio.find(section);
  return it != data.ratio.end() ? it->second : -1;
}

void User::SetSectionRatio(const std::string& section, int ratio)
{
  this->data.ratio[section] = ratio;
  db->SaveRatio();
}

long long User::SectionCredits(const std::string& section) const
{
  auto it = data.credits.find(section);
  return it != data.credits.end() ? it->second : -1;
}

void User::IncrSectionCredits(const std::string& section, long long kBytes)
{
  db->IncrCredits(section, kBytes);
}

bool User::DecrSectionCredits(const std::string& section, long long kBytes)
{
  return db->DecrCredits(section, kBytes, false);
}

void User::DecrSectionCreditsForce(const std::string& section, long long kBytes)
{
  (void) db->DecrCredits(section, kBytes, false);
}

void User::Purge() const
{
  db->Purge();
}

boost::optional<User> User::Load(acl::UserID uid)
{
  auto data = db::User::Load(uid);
  if (!data) return boost::optional<User>();
  return boost::optional<User>(User(std::move(*data)));
}

boost::optional<User> User::Load(const std::string& name)
{
  return Load(NameToUID(name));
}

boost::optional<User> User::Create(const std::string& name, const std::string& password, 
                                   acl::UserID creator)
{
  try
  {
    User user;
    user.data.name = name;
    user.data.creator = creator;
    user.data.id = user.db->Create();
    user.SetPassword(password);
    return boost::optional<User>(user);
  }
  catch (const db::DBKeyError&)
  {
    return boost::optional<User>();
  }
}

User User::FromTemplate(const std::string& name, const std::string& password,
                        acl::UserID creator, const User& templateUser)
{
  try
  {
    User user(templateUser);
    user.data.name = name;
    user.SetPassword(password);
    user.data.creator = creator;
    user.DelFlag(Flag::Template);
    user.data.id = user.db->Create();
    return user;
  }
  catch (const db::DBKeyError&)
  {
    throw util::RuntimeError("User already exists");
  }
}


std::vector<acl::UserID> User::GetUIDs(const std::string& multiStr)
{
  return db::GetUIDs(multiStr);
}

std::vector<acl::User> User::GetUsers(const std::string& multiStr)
{
  auto userData = db::GetUsers(multiStr);
  std::vector<acl::User> users;
  users.reserve(userData.size());
  for (auto& data : userData)
  {
    users.push_back(User(std::move(data)));
  }
  return users;
}

std::string UIDToName(acl::UserID uid)
{
  return db::UIDToName(uid);
}

acl::UserID NameToUID(const std::string& name)
{
  return db::NameToUID(name);
}

acl::GroupID UIDToPrimaryGID(acl::UserID uid)
{
  return db::UIDToPrimaryGID(uid);
}

} /* acl namespace */
