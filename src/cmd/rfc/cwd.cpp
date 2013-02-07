#include "cmd/rfc/cwd.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"
#include "util/status.hpp"

namespace cmd { namespace rfc
{

void CWDCommand::ShowDiz(const fs::VirtualPath& path)
{
  fs::RealPath real(fs::MakeReal(path));
  for (const auto& diz : cfg::Get().ShowDiz())
  {
    if (!diz.ACL().Evaluate(client.User())) continue;
    try
    {
      fs::RealPath dizPath = real / diz.Path();
      if (util::path::Status(dizPath.ToString()).IsReadable())
      {
        std::string lines;
        if (util::ReadFileToString(dizPath.ToString(), lines))
        {
          control.PartReply(ftp::FileActionOkay, lines);
        }
      }
    }
    catch (const util::SystemError&)
    {
    }
  }
}

void CWDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  util::Error e = fs::ChangeDirectory(client.User(),  path);
  if (e)
  {
    ShowDiz(path);
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
    return;
  }
  
  fs::VirtualPath match;
  if (e.Errno() == ENOENT && 
     (e = fs::ChangeAlias(client.User(), fs::Path(argStr), match)))
  {
    ShowDiz(path);
    control.Reply(ftp::FileActionOkay, "CWD command successful (Alias: " + 
          match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && 
      (e = fs::ChangeMatch(client.User(), path, match)))
  {
    ShowDiz(path);
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && 
      (e = fs::ChangeCdpath(client.User(), fs::Path(argStr), match)))
  {
    ShowDiz(path);
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;    
  }

  control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  throw cmd::NoPostScriptError();
}

} /* rfc namespace */
} /* cmd namespace */
