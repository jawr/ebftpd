#include "cmd/rfc/mdtm.hpp"
#include "acl/check.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void MDTMCommand::Execute()
{
  namespace PP = acl::PathPermission;

  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path absolute = client.WorkDir() / args[1];
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  const std::string& Sitepath = cfg::Get().Sitepath();
  fs::Path real = fs::Path(Sitepath) + absolute;
  
  fs::Status status;
  try
  {
    status.Reset(real);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
}

} /* rfc namespace */
} /* cmd namespace */
