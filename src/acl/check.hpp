#ifndef __ACL_CHECK_HPP
#define __ACL_CHECK_HPP

#include "util/error.hpp"
#include "ftp/client.hpp"

namespace acl
{
              
namespace Message
{

enum Type
{
  Welcome,
  Goodbye,
  Newsfile
};

template <Type type>
fs::Path Choose(const User& user);

}
              
} /* acl namespace */

#endif
