#include <cstring>

#if defined(__FreeBSD__)
# include <sys/extattr.h>
#else
# include <sys/xattr.h>
#endif

#include "fs/owner.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"

namespace fs
{

namespace
{

#if defined(__FreeBSD__)

int setxattr(const char *path, const char *name, const void *value, size_t size, int /* flags */)
{
  int ret = extattr_set_file(path, EXTATTR_NAMESPACE_USER, name, value, size);
  return ret >= 0 ? 0 : ret;
}

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
  return extattr_get_file(path, EXTATTR_NAMESPACE_USER, name, value, size);
}

#endif

const char* uidAttributeName = "user.ebftpd.uid";
const char* gidAttributeName = "user.ebftpd.gid";

int32_t GetAttribute(const std::string& path, const char* attribute)
{
  char buf[11];
  int len = getxattr(path.c_str(), attribute, buf, sizeof(buf));
  if (len < 0)
  {
    if (errno != ENOATTR)
    {
    std::cout << errno << " " << ENOATTR << std::endl;
      logs::error << "Error while reading filesystem attribute " << attribute 
                  << ": " << path << " : " << util::Error::Failure(errno).Message() << logs::endl;
    }
    return 0;
  }
  
  buf[len] = '\0';
  
  int32_t id;
  if (sscanf(buf, "%i", &id) != 1)
  {
    logs::error << "Invalid filesystem ownership attribute " << attribute 
                << ", resetting to 0: " << path << " : " << buf << logs::endl;
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
    logs::error << "Error while setting filesystem ownership attribute " << attribute 
                << ": " << path << " : " << e.Message() << logs::endl;
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

#ifndef EXTERNAL_TOOL
Owner GetOwner(const RealPath& path)
{
  return GetOwner(path.ToString());

}

util::Error SetOwner(const RealPath& path, const Owner& owner)
{
  return SetOwner(path.ToString(), owner);
}
#endif

} /* fs namespace */
