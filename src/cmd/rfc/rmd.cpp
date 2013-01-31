#include "cmd/rfc/rmd.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

void RMDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));
  util::Error e = fs::RemoveDirectory(client,  path);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), path))
    db::index::Delete(path.ToString());
  
  control.Reply(ftp::FileActionOkay, "RMD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
