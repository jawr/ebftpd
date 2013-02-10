#ifndef __ACL_PATH_HPP
#define __ACL_PATH_HPP

#include <string>
#include "util/error.hpp"

namespace fs
{
class VirtualPath;
class Path;
}

namespace acl
{ 

class User;

namespace path
{

enum Type
{
  Upload,
  Resume,
  Overwrite,
  Makedir,
  Download,
  Rename,
  Filemove,
  Nuke,
  Delete,
  View,
  Hideinwho,
  Freefile,
  Nostats,
  Hideowner
};

template <Type type>
util::Error Allowed(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error FileAllowed(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error DirAllowed(const User& user, const fs::VirtualPath& path);

util::Error Filter(const User& user, const fs::Path& basename, 
    fs::Path& messagePath);

} /* path namespace */
} /* acl namespace */

#endif
