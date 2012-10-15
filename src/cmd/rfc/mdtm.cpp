#include "cmd/rfc/mdtm.hpp"
#include "acl/check.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

cmd::Result MDTMCommand::Execute()
{
  namespace PP = acl::PathPermission;

  fs::Path absolute = client.WorkDir() / argStr;
  
  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "MDTM failed: " + e.Message());
    return cmd::Result::Okay;
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
    return cmd::Result::Okay;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
