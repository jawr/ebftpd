//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cstring>
#include "fs/owner.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"
#include "util/path/extattr.hpp"

namespace fs
{

namespace
{

const char* uidAttributeName = "user.ebftpd.uid";
const char* gidAttributeName = "user.ebftpd.gid";

int32_t GetAttribute(const std::string& path, const char* attribute)
{
  char buf[11];
  int len = getxattr(path.c_str(), attribute, buf, sizeof(buf));
  if (len < 0)
  {
    if (errno != ENOATTR && errno != ENODATA && errno != ENOENT)
    {
      logs::Error("Error while reading filesystem attribute %1%: %2%: %3%", 
                  attribute, path, util::Error::Failure(errno).Message());
    }
    return 0;
  }
  
  buf[len] = '\0';
  
  int32_t id;
  if (sscanf(buf, "%i", &id) != 1)
  {
    logs::Error("Invalid filesystem ownership attribute %1%, resetting to 0: %2%: %3%", 
                attribute, path, buf);
  }
  return id;
}

util::Error SetAttribute(const std::string& path, const char* attribute, int32_t id)
{
  char buf[11];
  int len = snprintf(buf, sizeof(buf), "%i", id);
  if (setxattr(path.c_str(), attribute, buf, len, 0) < 0)
  {
    auto e = util::Error::Failure(errno);
    logs::Error("Error while setting filesystem ownership attribute %1%: %2%: %3%", 
                attribute, path, e.Message());
    return e;
  }  
  return util::Error::Success();
}

}

Owner GetOwner(const std::string& path)
{
  return Owner(GetAttribute(path, uidAttributeName),
               GetAttribute(path, gidAttributeName));

}

util::Error SetOwner(const std::string& path, const Owner& owner)
{
  if (owner.UID() != -1)
  {
    auto e = SetAttribute(path, uidAttributeName, owner.UID());
    if (!e) return e;
  }
  
  if (owner.GID() != -1)
  {
    return SetAttribute(path, gidAttributeName, owner.GID());
  }
  
  return util::Error::Success();
}

Owner GetOwner(const RealPath& path)
{
  return GetOwner(path.ToString());

}

util::Error SetOwner(const RealPath& path, const Owner& owner)
{
  return SetOwner(path.ToString(), owner);
}

} /* fs namespace */
