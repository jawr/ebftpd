#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"

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
    isDirectory = fs::Status(client.User(), client.RenameFrom()).IsDirectory();
    if (isDirectory)
      e = fs::RenameDirectory(client.User(), client.RenameFrom(), path);
    else
      e = fs::RenameFile(client.User(), client.RenameFrom(), path);    
      
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
    // this should be changed to a single move action so as to retain the
    // creation date in the database
    if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), client.RenameFrom()))
      db::index::Delete(client.RenameFrom().ToString());

    if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), path))
    {
      auto section = cfg::Get().SectionMatch(path);
      db::index::Add(path.ToString(), section ? section->Name() : "");
    }
  }
  
  control.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

} /* rfc namespace */
} /* cmd namespace */
