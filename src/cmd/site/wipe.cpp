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

namespace cmd { namespace site
{

void WIPECommand::Process(const fs::Path& pathmask, int depth)
{
  using util::string::WildcardMatch;
  const cfg::Config& config = cfg::Get();
  fs::Path absolute = (client.WorkDir() / pathmask).Expand();
  try
  {
    fs::DirContainer dir(client, absolute.Dirname());
    for (auto& entry : dir)
    {
      if (!WildcardMatch(absolute.Basename(), entry))
        continue;

      fs::Path fullPath = (absolute.Dirname() / entry).Expand();
      fs::Path relative = (pathmask.Dirname() / entry).Expand();
      try
      {
        fs::Status status(config.Sitepath() + fullPath);
        if (status.IsDirectory())
        {
          if ((recursive || depth == 1) && !status.IsSymLink())
            Process((relative / "*").Expand(), depth + 1);
          util::Error e = fs::RemoveDirectory(client, fullPath);
          if (!e)
          {
            control.PartReply(ftp::CommandOkay, "WIPE " + 
                relative.ToString() + ": " + e.Message());
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
                relative.ToString() + ": " + e.Message());
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
            relative.ToString() + ": " + e.Message());        
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "WIPE " + absolute.Dirname().ToString() + ": " + e.Message());
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
    pathmaskStr = argStr.substr(2);
  }
  else
    pathmaskStr = argStr;
  
  boost::trim(pathmaskStr);
  return true;
}

cmd::Result WIPECommand::Execute()
{
  if (!ParseArgs()) return cmd::Result::SyntaxError;

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive wipe!");
    return cmd::Result::Okay;
  }

  Process(pathmaskStr);
  
  std::ostringstream os;
  os << "WIPE finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
