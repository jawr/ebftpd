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

util::Error Chmod(ftp::Client& client, const Path& path, const Mode& mode)
{
  namespace PP = acl::path;

  Path absolute = (client.WorkDir() / path).Expand();
  util::Error e = PP::FileAllowed<PP::View>(client.User(), absolute);
  if (!e) return e;
  
  Path real = cfg::Get().Sitepath() + absolute;
  mode_t userMask = umask(0);
  
  try
  {
    Status status(real);
    if (status.IsDirectory())
    {
      util::Error e = PP::DirAllowed<PP::View>(client.User(), absolute);
      if (!e) return e;
    }
    
    mode_t newMode;
    mode.Apply(status.Native().st_mode, userMask, newMode);
    
    if (chmod(real.CString(), newMode) < 0)
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
