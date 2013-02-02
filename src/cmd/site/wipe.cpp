#include <sstream>
#include <string>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/wipe.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "acl/path.hpp"

namespace cmd { namespace site
{

void WIPECommand::Process(fs::VirtualPath pathmask, int depth)
{
  using util::string::WildcardMatch;

  try
  {
    for (auto& entry : fs::DirContainer(client, pathmask.Dirname()))
    {
      if (!WildcardMatch(pathmask.Basename().ToString(), entry.ToString()))
        continue;

      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        fs::Status status(fs::MakeReal(entryPath));
        if (status.IsDirectory())
        {
          if ((recursive || depth == 1) && !status.IsSymLink())
            Process(entryPath / "*", depth + 1);
          util::Error e = fs::RemoveDirectory(client, entryPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + 
                entryPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
          {
            if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), entryPath))
              db::index::Delete(entryPath.ToString());
            ++dirs;
          }
        }
        else
        {
          util::Error e = fs::DeleteFile(client, entryPath);
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
  boost::to_lower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    patharg = argStr.substr(2);
  }
  else
    patharg = argStr;
  
  boost::trim(patharg);
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

  Process(fs::PathFromUser(patharg));
  
  std::ostringstream os;
  os << "WIPE finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
