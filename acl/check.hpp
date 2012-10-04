#ifndef __ACL_CHECK_HPP
#define __ACL_CHECK_HPP

#include "util/error.hpp"

namespace fs
{
class Path;
}

namespace acl
{

class User;

namespace PathPermission
{

enum Type
{
  Upload,
  Resume,
  Makedir,
  Download,
  Dirlog,
  Rename,
  Filemove,
  Nuke,
  Delete,
  View
};

template <Type type>
util::Error FileAllowed(const User& user, const std::string& path);

template <Type type>
util::Error DirAllowed(const User& user, std::string path);

} /* PathPermission namespace */

} /* acl namespace */

#endif
