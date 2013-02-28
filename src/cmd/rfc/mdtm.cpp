#include "cmd/rfc/mdtm.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "util/path/status.hpp"
#include "fs/path.hpp"

namespace cmd { namespace rfc
{

void MDTMCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  
  util::Error e(acl::path::FileAllowed<acl::path::View>(client.User(), path));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  util::path::Status status;
  try
  {
    status.Reset(fs::MakeReal(path).ToString());
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }

  if (!status.IsRegularFile())
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": Not a plain file.");
    return;
  }
  
  char timestamp[15];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S",
           localtime(&status.Native().st_mtime));
  control.Reply(ftp::FileStatus, timestamp);
}

} /* rfc namespace */
} /* cmd namespace */
