#include <sstream>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/wipe.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"
#include "fs/directory.hpp"
#include "fs/file.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void WIPECommand::Process(const fs::Path& absmask, int depth)
{
  using util::string::WildcardMatch;
  const cfg::Config& config = cfg::Get();
  try
  {
    fs::DirContainer dir(client, absmask.Dirname());
    for (auto& entry : dir)
    {
      if (!WildcardMatch(absmask.Basename(), entry))
        continue;

      fs::Path fullPath = (absmask.Dirname() / entry).Expand();
      try
      {
        fs::Status status(config.Sitepath() + fullPath);
        if (status.IsDirectory())
        {
          if ((recursive || depth == 1) && !status.IsSymLink())
            Process((fullPath / "*").Expand(), depth + 1);
          util::Error e = fs::RemoveDirectory(client, fullPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + 
                fullPath.ToString() + ": " + e.Message());
            ++failed;
          }
          else
            ++dirs;
        }
        else
        {
          util::Error e = fs::DeleteFile(client, fullPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " +
                fullPath.ToString() + ": " + e.Message());
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
            fullPath.ToString() + ": " + e.Message());        
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "WIPE " + absmask.ToString() + ": " + e.Message());
  }
}

bool WIPECommand::ParseArgs()
{
  int n = 1;
  boost::to_lower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    pathmask = argStr.substr(2);
  }
  else
    pathmask = argStr;
  
  boost::trim(pathmask);
  return true;
}

void WIPECommand::Execute()
{
  if (!ParseArgs()) throw cmd::SyntaxError();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive wipe!");
    return;
  }

  Process((client.WorkDir() / pathmask).Expand());
  
  std::ostringstream os;
  os << "WIPE finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
