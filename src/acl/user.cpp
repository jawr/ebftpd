#include <algorithm>
#include <cassert>
#include "acl/user.hpp"
#include "util/passwd.hpp"
#include "db/user.hpp"
#include "util/error.hpp"
#include "db/error.hpp"
#include "util/string.hpp"
#include "util/verify.hpp"
#include "db/userutil.hpp"
#include "db/user.hpp"
#include "util/scopeguard.hpp"
#include "acl/group.hpp"

namespace acl
{

UserData::UserData() :
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
  db(new db::User(data))
{
}

User::User(UserData&& data_) :
  data(data_),
  db(new db::User(data))
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
  db(new db::User(data))
{
}

User::User(const User& other) :
  data(other.data),
  db(new db::User(data))
{
}

User::~User()
{
}

bool User::Rename(const std::string& name)
{
  std::string oldName = data.name;
  data.name = name; 
  if (!db->SaveName())
  {
    data.name.swap(oldName);
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
  
  data.ipMasks.push_back(ipMask); 
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

std::string User::DelIPMask(size_t index)
{
  verify(index < data.ipMasks.size());
  std::string mask = *(data.ipMasks.begin() + index);
  data.ipMasks.erase(data.ipMasks.begin() + index);
  db->SaveIPMasks();
  return mask;
}

void User::ClearIPMasks()
{
  data.ipMasks.clear();
  db->SaveIPMasks();
}

bool User::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  return HexEncode(HashPassword(password, HexDecode(data.salt))) == data.password;
}

void User::SetPasswordNoSave(const std::string& password)
{
  using namespace util::passwd;  
  std::string rawSalt = GenerateSalt();
  data.password = HexEncode(HashPassword(password, rawSalt));
  data.salt = HexEncode(rawSalt);
}

void User::SetPassword(const std::string& password)
{
  auto trans = util::MakeTransaction(data.password);
  SetPasswordNoSave(password);
  db->SavePassword();
}

void User::SetFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  auto trans = util::MakeTransaction(data.flags, flags);
  db->SaveFlags();
}

void User::AddFlags(const std::string& flags)
{
  assert(ValidFlags(flags));
  auto trans = util::MakeTransaction(data.flags);
  for (char ch: flags)
  {
    if (data.flags.find(ch) == std::string::npos) data.flags += ch;
  }
  
  std::sort(data.flags.begin(), data.flags.end());
  db->SaveFlags();
}

void User::AddFlag(Flag flag)
{
  AddFlags(std::string(1, static_cast<char>(flag)));
}

void User::DelFlags(const std::string& flags)
{
  auto trans = util::MakeTransaction(data.flags);
  for (char ch: flags)
  {
    std::string::size_type pos = data.flags.find(ch);
    if (pos != std::string::npos) data.flags.erase(pos, 1);
  }
  db->SaveFlags();
}

void User::DelFlag(Flag flag)
{
  DelFlags(std::string(1, static_cast<char>(flag)));
}

bool User::HasFlag(Flag flag) const
{
  return data.flags.find(static_cast<char>(flag)) != std::string::npos;
}

std::string User::PrimaryGroup() const
{
  return GIDToName(data.primaryGid);
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
  auto trans1 = util::MakeTransaction(data.primaryGid);
  auto trans2 = util::MakeTransaction(data.secondaryGids);

  if (data.primaryGid == gid) return;
  if (data.primaryGid != -1)
  {
    data.secondaryGids.insert(data.secondaryGids.begin(), data.primaryGid);
  }
  
  data.primaryGid = gid;
  db->SaveGIDs();
}

void User::AddGIDs(const std::vector<acl::GroupID>& gids)
{
  if (gids.empty()) return;
  
  auto trans1 = util::MakeTransaction(data.primaryGid);
  auto trans2 = util::MakeTransaction(data.secondaryGids);

  size_t offset = 0;
  if (data.primaryGid == -1)
  {
    data.primaryGid = gids.front();
    ++offset;
  }
  
  if (gids.size() > offset)
  {
    std::copy_if(gids.begin() + offset, gids.end(), std::back_inserter(data.secondaryGids),
        [&](acl::GroupID gid)
        {
          return std::find(data.secondaryGids.begin(), data.secondaryGids.end(), gid) == 
                data.secondaryGids.end() && gid != data.primaryGid;
        });
  }
  
  db->SaveGIDs();
}

void User::CleanGadminGIDs()
{
  auto end = data.gadminGids.end();
  for (auto it = data.gadminGids.begin(); it != end; )
  {
    if (*it != data.primaryGid &&
        std::find(data.secondaryGids.begin(), data.secondaryGids.end(), *it) ==
        data.secondaryGids.end())
    {
      data.gadminGids.erase(it++);
    }
    else
    {
      ++it;
    }
  }
}

void User::DelGIDs(const std::vector<acl::GroupID>& gids)
{
  auto trans1 = util::MakeTransaction(data.primaryGid);
  auto trans2 = util::MakeTransaction(data.secondaryGids);
  auto trans3 = util::MakeTransaction(data.gadminGids);
  
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
  }

  CleanGadminGIDs();
  db->SaveGIDs();
}

void User::SetGIDs(const std::vector<acl::GroupID>& gids)
{
  auto trans1 = util::MakeTransaction(data.primaryGid);
  auto trans2 = util::MakeTransaction(data.secondaryGids);
  auto trans3 = util::MakeTransaction(data.gadminGids);

  data.primaryGid = -1;
  data.secondaryGids.clear();
  
  if (!gids.empty())
  {
    data.primaryGid = gids.front();
    data.secondaryGids.assign(gids.begin() + 1, gids.end());
  }
  
  CleanGadminGIDs();
  db->SaveGIDs();
}

void User::ToggleGIDs(const std::vector<acl::GroupID>& gids)
{
  auto trans1 = util::MakeTransaction(data.primaryGid);
  auto trans2 = util::MakeTransaction(data.secondaryGids);
  auto trans3 = util::MakeTransaction(data.gadminGids);

  std::vector<acl::GroupID> diffGids;
  
  if (data.primaryGid != -1)
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
  
  if (diffGids.empty())
  {
    data.primaryGid = -1;
    data.secondaryGids.clear();
  }
  else
  {
    data.primaryGid = diffGids.front();
    data.secondaryGids.assign(diffGids.begin() + 1, diffGids.end());
  }

  CleanGadminGIDs();
  db->SaveGIDs();
}

bool User::HasGadminGID(GroupID gid) const
{
  return std::find(data.gadminGids.begin(), data.gadminGids.end(), gid) != data.gadminGids.end();
}

void User::AddGadminGID(GroupID gid)
{
  auto trans = util::MakeTransaction(data.gadminGids);
  data.gadminGids.insert(gid);
  db->SaveGadminGIDs();
  if (!HasFlag(Flag::Gadmin)) AddFlag(Flag::Gadmin);
}

void User::DelGadminGID(GroupID gid)
{
  auto trans = util::MakeTransaction(data.gadminGids);
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
  auto trans = util::MakeTransaction(data.weeklyAllotment, weeklyAllotment);
  db->SaveWeeklyAllotment();
}

void User::SetHomeDir(const std::string& homeDir)
{
  auto trans = util::MakeTransaction(data.homeDir, homeDir);
  db->SaveHomeDir();
}

void User::SetIdleTime(int idleTime)
{
  auto trans = util::MakeTransaction(data.idleTime, idleTime);
  db->SaveIdleTime();
}

bool User::Expired() const
{
  if (!data.expires) return false;
  return boost::gregorian::day_clock::local_day() >= *data.expires;
}

void User::SetExpires(const boost::optional<boost::gregorian::date>& expires)
{
  auto trans = util::MakeTransaction(data.expires, expires);
  db->SaveExpires();
}

void User::SetNumLogins(int numLogins)
{
  auto trans = util::MakeTransaction(data.numLogins, numLogins);
  db->SaveNumLogins();
}

void User::SetComment(const std::string& comment)
{
  auto trans = util::MakeTransaction(data.comment, comment);
  db->SaveComment();
}

void User::SetTagline(const std::string& tagline)
{
  auto trans = util::MakeTransaction(data.tagline, tagline);
  db->SaveTagline();
}

void User::SetMaxDownSpeed(long long maxDownSpeed)
{
  auto trans = util::MakeTransaction(data.maxDownSpeed, maxDownSpeed);
  db->SaveMaxDownSpeed();
}

void User::SetMaxUpSpeed(long long maxUpSpeed)
{
  auto trans = util::MakeTransaction(data.maxUpSpeed, maxUpSpeed);
  db->SaveMaxUpSpeed();
}

void User::SetMaxSimDown(int maxSimDown)
{
  auto trans = util::MakeTransaction(data.maxSimDown, maxSimDown);
  db->SaveMaxSimDown();
}

void User::SetMaxSimUp(int maxSimUp)
{
  auto trans = util::MakeTransaction(data.maxSimUp, maxSimUp);
  db->SaveMaxSimUp();
}

void User::SetLoggedIn()
{
  auto trans1 = util::MakeTransaction(data.loggedIn, data.loggedIn + 1);
  db->SaveLoggedIn();

  auto trans2 = util::MakeTransaction(data.lastLogin,
                  boost::posix_time::microsec_clock::local_time());
  db->SaveLastLogin();
}

int User::SectionRatio(const std::string& section) const
{
  auto it = data.ratio.find(section);
  return it != data.ratio.end() ? it->second : -1;
}

void User::SetSectionRatio(const std::string& section, int ratio)
{
  auto trans = util::MakeTransaction(data.ratio);
  data.ratio[section] = ratio;
  db->SaveRatio();
}

long long User::SectionCredits(const std::string& section) const
{
  auto it = data.credits.find(section);
  return it != data.credits.end() ? it->second : 0;
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
  (void) db->DecrCredits(section, kBytes, true);
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
  auto data = db::User::Load(name);
  if (!data) return boost::optional<User>();
  return boost::optional<User>(User(std::move(*data)));
}

boost::optional<User> User::Create(const std::string& name, 
        const std::string& password, acl::UserID creator)
{
  User user;
  user.data.name = name;
  user.data.creator = creator;
  user.SetPasswordNoSave(password);
  if (!user.db->Create()) return boost::optional<User>();
  return boost::optional<User>(user);
}

boost::optional<User> User::FromTemplate(const std::string& name, 
        const std::string& password, acl::UserID creator, const User& templateUser)
{
  User user(templateUser);
  user.data.id = -1;
  user.data.name = name;
  user.data.creator = creator;
  user.SetPasswordNoSave(password);
  user.DelFlag(Flag::Template);
  if (!user.db->Create()) return boost::optional<User>();
  return boost::optional<User>(user);
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
