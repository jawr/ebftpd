#include "cmd/rfc/rnto.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "cfg/get.hpp"
#include "util/path/status.hpp"
#include "fs/path.hpp"

namespace cmd { namespace rfc
{

void RNTOCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  util::Error e(acl::path::Filter(client.User(), path.Basename()));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "Path name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }

  bool isDirectory;
  try
  {
    isDirectory = util::path::Status(client.RenameFrom().ToString()).IsDirectory();
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
    if (cfg::Get().IsIndexed(client.RenameFrom().ToString()))
      db::index::Delete(client.RenameFrom().ToString());

    if (cfg::Get().IsIndexed(path.ToString()))
    {
      auto section = cfg::Get().SectionMatch(path.ToString());
      db::index::Add(path.ToString(), section ? section->Name() : "");
    }
  }
  
  control.Reply(ftp::FileActionOkay, "RNTO command successful.");
}

} /* rfc namespace */
} /* cmd namespace */
