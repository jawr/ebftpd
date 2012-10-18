#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include "acl/check.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

#include <iostream>

namespace acl
{

namespace Message
{

std::string Evaluate(const std::vector<cfg::setting::Right>& rights, 
                     const User& user)
{
  for (const auto& right : rights)
  {
    if (right.ACL().Evaluate(user)) return right.Path();
  }
  return "";
}

template <Type type>
struct Traits;

template <>
struct Traits<Welcome>
{
  static std::string Choose(const User& user)
  {
    return Evaluate(cfg::Get().WelcomeMsg(), user);
  }
};

template <>
struct Traits<Goodbye>
{
  static std::string Choose(const User& user)
  {
    return Evaluate(cfg::Get().GoodbyeMsg(), user);
  }
};

template <>
struct Traits<Newsfile>
{
  static std::string Choose(const User& user)
  {
    return Evaluate(cfg::Get().Newsfile(), user);
  }
};

template <Type type>
std::string Chooose(const User& user)
{
  return Traits<type>::Choose(user);
}

}
  
} /* acl namespace */


#ifdef ACL_CHECK_TEST

int main()
{
  using namespace acl;
  User user("test", "test", "1");
  std::string path("/hello/there");

  std::cout << (path + "/") << std::endl;
  
  std::cout << PathPermission::FileAllowed<PathPermission::Upload>(user, path) << std::endl;
  std::cout << PathPermission::DirAllowed<PathPermission::Makedir>(user, path) << std::endl;
  

}

#endif
