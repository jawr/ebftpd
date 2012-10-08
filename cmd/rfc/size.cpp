#include "cmd/rfc/size.hpp"
#include "acl/check.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void SIZECommand::Execute()
{
  namespace PP = acl::PathPermission;

  fs::Path absolute = (client.WorkDir() / argStr).Expand();

  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 2: " + e.Message());
    return;
  }
  
  fs::Status status;
  try
  {
    status.Reset(cfg::Get().Sitepath() + absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 1: " + e.Message());
    return;
  }
  
  control.Reply(ftp::FileStatus, boost::lexical_cast<std::string>(status.Size())); 
}

} /* rfc namespace */
} /* cmd namespace */
