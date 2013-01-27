#ifndef __ACL_CHECK_HPP
#define __ACL_CHECK_HPP

#include "util/error.hpp"
#include "ftp/client.hpp"

namespace acl { namespace message
{

enum Type
{
  Welcome,
  Goodbye,
  Newsfile
};

template <Type type>
fs::Path Choose(const User& user);

} /* message namespace */
} /* acl namespace */

#endif
