#include "cmd/rfc/rnfr.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void RNFRCommand::Execute()
{
  namespace PP = acl::path;
  
  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  
  util::Error e(PP::FileAllowed<PP::Rename>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }

  try
  {
    fs::Status status(cfg::Get().Sitepath() + absolute);
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  client.SetRenameFrom(absolute);
  control.Reply(ftp::PendingMoreInfo, "File exists, ready for destination name."); 
  return;
}

} /* rfc namespace */
} /* cmd namespace */
