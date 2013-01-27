#ifndef __ACL_MISC_HPP
#define __ACL_MISC_HPP

#include "util/error.hpp"
#include "ftp/client.hpp"

namespace acl { namespace message
{

enum Type
{
  Welcome,
  Goodbye
};

template <Type type>
fs::Path Choose(const User& user);

} /* message namespace */

namespace stats
{

int MaxUsers(const User& user);
int MaxGroups(const User& user);

} /* stats namespace */

} /* acl namespace */

#endif
