#include <cassert>
#include <boost/algorithm/string/predicate.hpp>
#include "acl/message.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

namespace acl { namespace message
{

fs::Path Evaluate(const std::vector<cfg::setting::Right>& rights, const User& user)
{
  for (const auto& right : rights)
  {
    if (right.ACL().Evaluate(user)) return right.Path();
  }
  return fs::Path();
}

template <Type type>
struct Traits;

template <>
struct Traits<Welcome>
{
  static fs::Path Choose(const User& user)
  {
    return Evaluate(cfg::Get().WelcomeMsg(), user);
  }
};

template <>
struct Traits<Goodbye>
{
  static fs::Path Choose(const User& user)
  {
    return Evaluate(cfg::Get().GoodbyeMsg(), user);
  }
};

template <Type type>
fs::Path Choose(const User& user)
{
  return Traits<type>::Choose(user);
}

template fs::Path Choose<Welcome>(const User& user);
template fs::Path Choose<Goodbye>(const User& user);

} /* message namespace */
} /* acl namespace */
