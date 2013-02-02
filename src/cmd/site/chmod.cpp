#include <sstream>
#include <string>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/site/chmod.hpp"
#include "fs/chmod.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"

namespace cmd { namespace site
{

void CHMODCommand::Process(fs::VirtualPath pathmask)
{
  using util::string::WildcardMatch;
  
  try
  {  
    for (auto& entry : fs::DirContainer(client.User(), pathmask.Dirname()))
    {
      if (!WildcardMatch(pathmask.Basename().ToString(), entry.ToString()))
        continue;

      fs::VirtualPath entryPath(pathmask.Dirname() / entry);
      try
      {
        fs::Status status(fs::MakeReal(entryPath));          
        util::Error e = fs::Chmod(client.User(), entryPath, *mode);
        if (!e)
        {
          ++failed;
          control.PartReply(ftp::CommandOkay, "CHOWN " + 
              entryPath.ToString() + ": " + e.Message());        
        }
        else
        if (status.IsDirectory())
        {
          ++dirs;
          if (recursive && !status.IsSymLink()) 
            Process(entryPath / "*");
        }
        else ++files;
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
        "CHMOD " + pathmask.Dirname().ToString() + ": " + e.Message());
  }
}

void CHMODCommand::ParseArgs()
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
  if (pos == std::string::npos) throw cmd::SyntaxError();
  
  patharg = argStr.substr(pos);
  boost::trim(patharg);
}

void CHMODCommand::Execute()
{
  ParseArgs();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chmod!");
    return;
  }

  try
  {
    mode.reset(fs::Mode(modeStr));
  }
  catch (const fs::InvalidModeString& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  Process(fs::PathFromUser(patharg));
  
  std::ostringstream os;
  os << "CHMOD finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
