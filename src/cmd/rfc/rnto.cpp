#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"

namespace cmd { namespace rfc
{

void RNTOCommand::Execute()
{
  namespace PP = acl::path;
  
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::Path messagePath;
  util::Error e(acl::path::Filter(client.User(), path.Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
    control.Reply(ftp::ActionNotOkay, "Path name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }

  bool isDirectory;
  try
  {
    isDirectory = fs::Status(client, client.RenameFrom()).IsDirectory();
    if (isDirectory)
      e = fs::RenameDirectory(client, client.RenameFrom(), path);
    else
      e = fs::RenameFile(client, client.RenameFrom(), path);    
      
    if (!e)
    {
      control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
      throw cmd::NoPostScriptError();
    }
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }

  if (isDirectory)
  {
    if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), client.RenameFrom()))
      db::index::Delete(client.RenameFrom().ToString());

    if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), path))
      db::index::Add(path.ToString());
  }
  
  control.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

} /* rfc namespace */
} /* cmd namespace */
