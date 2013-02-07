#include <algorithm>
#include <cctype>
#include <bitset>
#include <sys/stat.h>
#include "fs/chmod.hpp"
#include "acl/user.hpp"
#include "fs/mode.hpp"
#include "util/status.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/path.hpp"

namespace fs
{

util::Error Chmod(const RealPath& path, const Mode& mode)
{
  try
  {
    mode_t newMode;
    mode.Apply(util::path::Status(path.ToString()).Native().st_mode, umask(0), newMode);
  
    if (chmod(MakeReal(path).CString(), newMode) < 0)
      return util::Error::Failure(errno);
  }
  catch (const util::SystemError& e)
  { return util::Error::Failure(e.Errno()); }
  
  return util::Error::Success();
}

util::Error Chmod(const acl::User& user, const VirtualPath& path, const Mode& mode)
{
  namespace PP = acl::path;

  util::Error e = PP::FileAllowed<PP::View>(user, path);
  if (!e) return e;
  
  mode_t userMask = umask(0);
  
  try
  {
    util::path::Status status(MakeReal(path).ToString());
    if (status.IsDirectory())
    {
      util::Error e = PP::DirAllowed<PP::View>(user, path);
      if (!e) return e;
    }
    
    mode_t newMode;
    mode.Apply(status.Native().st_mode, userMask, newMode);
    
    if (chmod(MakeReal(path).CString(), newMode) < 0)
      return util::Error::Failure(errno);
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  return util::Error::Success();
}

} /* fs namespace */
