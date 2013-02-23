#include <sstream>
#include <iomanip>
#include "cmd/site/diskfree.hpp"
#include "util/path/status.hpp"
#include "acl/path.hpp"
#include "fs/path.hpp"

namespace cmd { namespace site
{

void DISKFREECommand::Execute()
{
  std::string pathStr = argStr.empty() ? "." : argStr;
  fs::VirtualPath path(fs::PathFromUser(pathStr));
  if (!acl::path::DirAllowed<acl::path::View>(client.User(), path))
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ":" + util::Error::Failure(EACCES).Message());
    return;
  }
  
  unsigned long long bytes;
  auto e = util::path::FreeDiskSpace(fs::MakeReal(path).ToString(), bytes);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, pathStr + ":" + e.Message());
    return;
  }
  
  std::ostringstream os;
  os << "Disk free: " << std::fixed << std::setprecision(2) << (bytes / 1024 / 1024.0) << "MB";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
