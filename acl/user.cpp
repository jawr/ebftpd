#include <algorithm>
#include <iterator>
#include "util/passwd.hpp"
#include "acl/user.hpp"

namespace acl
{

User::User(const std::string& name,
           const std::string& password,
           const std::string& flags) :
  name(name),
  uid(69),
  primaryGid(69)
{
  SetPassword(password);
  AddFlags(flags);
}

void User::SetPassword(const std::string& password)
{
  using namespace util::passwd;
  
  std::string rawSalt = GenerateSalt();
  this->password = HexEncode(HashPassword(password, rawSalt));
  salt = HexEncode(rawSalt);
}

bool User::VerifyPassword(const std::string& password) const
{
  using namespace util::passwd;
  
  return HexEncode(HashPassword(password, HexDecode(salt))) == this->password;
}

void User::AddFlags(const std::string& flags)
{
  for (std::string::const_iterator it = flags.begin();
       it != flags.end(); ++it)
  {
    if (this->flags.find(*it) == std::string::npos) this->flags += *it;
  }
  
  std::sort(this->flags.begin(), this->flags.end());
}

void User::DelFlags(const std::string& flags)
{
  for (std::string::const_iterator it = flags.begin();
       it != flags.end(); ++it)
  {
    std::string::size_type pos = this->flags.find(*it);
    if (pos != std::string::npos) this->flags.erase(pos, 1);
  }  
}

bool User::CheckFlags(const std::string& flags) const
{
  for (std::string::const_iterator it =
       flags.begin(); it != flags.end(); ++it)
  {
    if (this->flags.find(*it) != std::string::npos) return true;
  }
  return false;
}

bool User::CheckFlag(Flag flag) const
{
  return this->flags.find(flag) != std::string::npos;
}

void User::AddSecondaryGID(gid_t gid)
{
  secondaryGids.insert(gid);
}

void User::DelSecondaryGID(gid_t gid)
{
  secondaryGids.erase(gid);
}

bool User::CheckGID(gid_t gid)
{
  return primaryGid == gid || secondaryGids.find(gid) != secondaryGids.end();
}

} /* acl namespace */

#ifdef ACL_USER_TEST

#include <iostream>
#include <mongo/client/dbclient.h>

int main()
{
  using namespace acl;
  
  User u("bioboy", "password", "1");
  
  std::cout << u.VerifyPassword("test1234") << std::endl;
  std::cout << u.VerifyPassword("password") << std::endl;
  u.SetPassword("wowsers");
  std::cout << "pass changed" << std::endl;
  std::cout << u.VerifyPassword("wowsers") << std::endl;
  std::cout << u.VerifyPassword("w0000000000t") << std::endl;
  
  
  std::cout << u.Flags() << std::endl;
  u.AddFlags("789");
  std::cout << u.Flags() << std::endl;
  u.DelFlags("17");
  std::cout << u.Flags() << std::endl;

  try
  {
    mongo::DBClientConnection c;
    c.connect("localhost");
    c.insert("ftpd.users", u.ToBSON());
  }
  catch (const mongo::DBException& e)
  {
    std::cout << "db connect: " << e.what() << std::endl;
  }
}

#endif
