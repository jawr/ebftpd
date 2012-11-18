#include <algorithm>
#include <cctype>
#include <bitset>
#include <sys/stat.h>
#include "fs/chmod.hpp"
#include "ftp/client.hpp"
#include "fs/mode.hpp"
#include "fs/status.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"

namespace fs
{

util::Error Chmod(const RealPath& path, const Mode& mode)
{
  try
  {
    mode_t newMode;
    mode.Apply(Status(path).Native().st_mode, umask(0), newMode);
  
    if (chmod(MakeReal(path).CString(), newMode) < 0)
      return util::Error::Failure(errno);
  }
  catch (const util::SystemError& e)
  { return util::Error::Failure(e.Errno()); }
  
  return util::Error::Success();
}

util::Error Chmod(ftp::Client& client, const VirtualPath& path, const Mode& mode)
{
  namespace PP = acl::path;

  util::Error e = PP::FileAllowed<PP::View>(client.User(), path);
  if (!e) return e;
  
  mode_t userMask = umask(0);
  
  try
  {
    Status status(MakeReal(path));
    if (status.IsDirectory())
    {
      util::Error e = PP::DirAllowed<PP::View>(client.User(), path);
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

#ifndef __FS_CHMOD_TEST

#include <iostream>
/*
int main()
{
  using namespace fs;
  mode_t mode;
  util::Error e = ModeFromString("0655", mode);
  if (!e) std::cout << "error: " << e.Message() << std::endl;
  else std::cout << mode << std::endl;
}*/

#endif
