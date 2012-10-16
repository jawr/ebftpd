#include <sstream>
#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/chmod.hpp"
#include "fs/chmod.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void CHMODCommand::Process(const std::string& pathmask, int depth)
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
      fs::Status status(config.Sitepath() + fullPath);
        
      util::Error e = fs::Chmod(client, fullPath, *mode);
      if (!e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, 
            "CHMOD " + entry + ": " + e.Message());
      }
      else
      if (status.IsDirectory())
      {
        ++dirs;
        if (recursive && !status.IsSymLink()) 
          Process(fullPath / "*", depth + 1);
      }
      else ++files;
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "CHMOD " + absolute.Dirname().ToString() + ": " + e.Message());
  }
}

bool CHMODCommand::ParseArgs()
{
  int n = 1;
  boost::to_lower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    boost::to_lower(args[n]);
  }
  
  modeStr = args[n];
  
  std::string::size_type pos =
      util::string::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) return false;
  
  pathmask = argStr.substr(pos);
  boost::trim(pathmask);
  return true;
}

// SITE CHMOD [-R] <MODE> <PATHMASK.. ..>
cmd::Result CHMODCommand::Execute()
{
  if (!ParseArgs()) return cmd::Result::SyntaxError;

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chmod!");
    return cmd::Result::Okay;
  }

  try
  {
    mode.reset(fs::Mode(modeStr));
  }
  catch (const fs::InvalidModeString& e)
  {
    control.Reply(ftp::ActionNotOkay, 
        "Chmod failed: " + e.Message());
    return cmd::Result::Okay;
  }

  Process(pathmask);
  
  std::ostringstream os;
  os << "CHMOD finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
