#include <cstring>
#include <sys/xattr.h>
#include "fs/owner.hpp"
#include "util/error.hpp"
#include "logs/logs.hpp"

namespace fs
{

namespace
{

const char* uidAttributeName = "user.ebftpd.uid";
const char* gidAttributeName = "user.ebftpd.gid";

int32_t GetAttribute(const RealPath& path, const char* attribute)
{
  char buf[11];
  int len = getxattr(path.CString(), attribute, buf, sizeof(buf));
  if (len < 0)
  {
    if (errno != ENODATA)
    {
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

util::Error SetAttribute(const RealPath& path, const char* attribute, int32_t id)
{
  char buf[11];
  int len = snprintf(buf, sizeof(buf), "%i", id);
  if (setxattr(path.CString(), attribute, buf, len, 0) < 0)
  {
    auto e = util::Error::Failure(errno);
    logs::error << "Error while setting filesystem ownership attribute " << attribute 
                << ": " << path << " : " << e.Message() << logs::endl;
    return e;
  }  
  return util::Error::Success();
}

}

Owner GetOwner(const RealPath& path)
{
  return Owner(GetAttribute(path, uidAttributeName),
               GetAttribute(path, gidAttributeName));

}

util::Error SetOwner(const RealPath& path, const Owner& owner)
{
  auto e = SetAttribute(path, uidAttributeName, owner.UID());
  if (!e) return e;
  
  return SetAttribute(path, gidAttributeName, owner.GID());
}

} /* fs namespace */
