#include <sstream>
#include <string>
#include "cmd/site/wipe.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"
#include "util/path/status.hpp"
#include "cfg/get.hpp"
#include "util/enumbitwise.hpp"
#include "acl/group.hpp"
#include "logs/logs.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void WIPECommand::Process(fs::VirtualPath pathmask)
{
  auto flags = fs::GlobIterator::NoFlags;
  if (recursive) flags |= fs::GlobIterator::Recursive;
  
  try
  {
    for (auto& entry : fs::GlobContainer(client.User(), pathmask, flags))
    {
      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        util::path::Status status(fs::MakeReal(entryPath).ToString());
        if (status.IsDirectory())
        {
          util::Error e = fs::RemoveDirectory(client.User(), entryPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + 
                entryPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
          {
            if (cfg::Get().IsIndexed(entryPath.ToString()))
              db::index::Delete(entryPath.ToString());
            ++dirs;
          }
        }
        else
        {
          util::Error e = fs::DeleteFile(client.User(), entryPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " +
                entryPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
            ++files;
        }
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "CHOWN " + 
            entryPath.ToString() + ": " + e.Message());        
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "WIPE " + pathmask.ToString() + ": " + e.Message());
  }
}

void WIPECommand::ParseArgs()
{
  int n = 1;
  util::ToLower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    patharg = argStr.substr(2);
  }
  else
    patharg = argStr;
  
  util::Trim(patharg);
}

void WIPECommand::Execute()
{
  ParseArgs();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive wipe!");
    throw cmd::NoPostScriptError();
  }

  auto path = fs::PathFromUser(patharg);
  Process(path);
  
  std::ostringstream os;
  os << "WIPE finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
  
  logs::Event(recursive ? "WIPE-r" : "WIPE", path, client.User().Name(),
              client.User().PrimaryGroup(), client.User().Tagline());
}

} /* site namespace */
} /* cmd namespace */
