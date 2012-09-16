#include <algorithm>
#include <iterator>
#include "util/passwd.hpp"
#include "user.hpp"

namespace acl
{

User::User(const std::string& username,
           const std::string& password,
           const std::string& flags) :
  username(username),
  uid(-1),
  primaryGid(-1)
{
  Password(password);
  AddFlags(flags);
}

void User::Password(const std::string& password)
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

} /* acl namespace */

#ifdef ACL_USER_TEST

#include <iostream>

int main()
{
  using namespace acl;
  
  User u("bioboy", "password", "1");
  
  std::cout << u.VerifyPassword("test1234") << std::endl;
  std::cout << u.VerifyPassword("password") << std::endl;
  u.Password("wowsers");
  std::cout << "pass changed" << std::endl;
  std::cout << u.VerifyPassword("wowsers") << std::endl;
  std::cout << u.VerifyPassword("w0000000000t") << std::endl;
  
  
  std::cout << u.Flags() << std::endl;
  u.AddFlags("789");
  std::cout << u.Flags() << std::endl;
  u.DelFlags("17");
  std::cout << u.Flags() << std::endl;
  
}

#endif
