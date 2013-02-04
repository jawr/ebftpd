#include <cassert>
#include <algorithm>
#include <iterator>
#include "util/passwd.hpp"
#include "acl/user.hpp"
#include "acl/types.hpp"
#include "db/user/user.hpp"
#include "util/string.hpp"

namespace acl
{

User::User(const std::string& name,
           const std::string& password,
           const std::string& flags) :
  modified(boost::posix_time::microsec_clock::local_time()),
  name(name),
  uid(-1),
  primaryGid(-1)
{
  SetPassword(password);
  AddFlags(flags);
}

void User::SetPassword(const std::string& password)
{
  using namespace util::passwd;
  
  modified = boost::posix_time::microsec_clock::local_time();
  std::string rawSalt = GenerateSalt();
  this->password = HexEncode(HashPassword(password, rawSalt));
  salt = HexEncode(rawSalt);
}

bool User::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  
  return HexEncode(HashPassword(password, HexDecode(salt))) == this->password;
}

void User::SetFlags(const std::string& flags)
{
  modified = boost::posix_time::microsec_clock::local_time();
  assert(ValidFlags(flags));
  this->flags = flags;
}

void User::AddFlags(const std::string& flags)
{
  modified = boost::posix_time::microsec_clock::local_time();
  assert(ValidFlags(flags));
  for (char ch: flags)
  {
    if (this->flags.find(ch) == std::string::npos) this->flags += ch;
  }
  
  std::sort(this->flags.begin(), this->flags.end());
}

void User::AddFlag(Flag flag)
{
  AddFlags(std::string(1, static_cast<char>(flag)));
}

void User::DelFlags(const std::string& flags)
{
  modified = boost::posix_time::microsec_clock::local_time();
  for (char ch: flags)
  {
    std::string::size_type pos = this->flags.find(ch);
    if (pos != std::string::npos) this->flags.erase(pos, 1);
  }  
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

void User::SetPrimaryGID(GroupID primaryGid)
{
  modified = boost::posix_time::microsec_clock::local_time();
  assert(primaryGid >= -1);
  this->primaryGid = primaryGid;
}

void User::AddSecondaryGID(GroupID gid)
{
  modified = boost::posix_time::microsec_clock::local_time();
  secondaryGids.emplace_back(gid);
}

void User::DelSecondaryGID(GroupID gid)
{
  modified = boost::posix_time::microsec_clock::local_time();
  secondaryGids.erase(std::remove(secondaryGids.begin(), secondaryGids.end(), gid), 
      secondaryGids.end());
}

void User::ResetSecondaryGIDs()
{
  modified = boost::posix_time::microsec_clock::local_time();
  secondaryGids.clear();
}

bool User::HasSecondaryGID(GroupID gid) const
{
  return std::find(secondaryGids.begin(), secondaryGids.end(), gid) != secondaryGids.end();
}

bool User::HasGID(GroupID gid) const
{
  return primaryGid == gid || HasSecondaryGID(gid);
}

void User::AddGadminGID(GroupID gid)
{
  modified = boost::posix_time::microsec_clock::local_time();
  if (!CheckFlag(Flag::Gadmin)) AddFlag(Flag::Gadmin);
  gadminGids.emplace_back(gid);
}

void User::DelGadminGID(GroupID gid)
{
  modified = boost::posix_time::microsec_clock::local_time();
  gadminGids.erase(std::remove(gadminGids.begin(), gadminGids.end(), gid), 
      gadminGids.end());
  if (gadminGids.empty()) DelFlag(Flag::Gadmin);

}

bool User::HasGadminGID(GroupID gid) const
{
  return std::find(gadminGids.begin(), gadminGids.end(), gid) != gadminGids.end();
}

util::Error User::AddIPMask(const std::string& mask, std::vector<std::string>& redundant)
{
  for (const std::string& m : ipMasks)
  {
    if (util::string::WildcardMatch(m, mask, false))
      return util::Error::Failure("Broader mask already exists");
  }

  for (auto it = ipMasks.begin(); it != ipMasks.end();)
  {
    if (util::string::WildcardMatch(mask, *it, false))
    {
      redundant.emplace_back(*it);
      it = ipMasks.erase(it);
    }
    else
      ++it;
  }
  
  if (ipMasks.size() >= 10)
  {
    return util::Error::Failure("Maximum of 10 IP masks reached");
  }
  
  modified = boost::posix_time::microsec_clock::local_time();  
  ipMasks.emplace_back(mask);
  
  return util::Error::Success();
}

util::Error User::DelIPMask(decltype(ipMasks.size()) index, std::string& deleted)
{
  if (index >= ipMasks.size()) return util::Error::Failure("IP mask index out of range");
  modified = boost::posix_time::microsec_clock::local_time();  
  deleted = *(ipMasks.begin() + index);
  ipMasks.erase(ipMasks.begin() + index);
  return util::Error::Success();
}

void User::DelAllIPMasks(std::vector<std::string>& deleted)
{
  modified = boost::posix_time::microsec_clock::local_time();  
  deleted = std::move(ipMasks);
  assert(ipMasks.empty());
}

} /* acl namespace */
