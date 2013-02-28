#include "cmd/rfc/cwd.hpp"
#include "fs/directory.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "util/misc.hpp"
#include "util/path/status.hpp"
#include "fs/path.hpp"
#include "acl/user.hpp"
#include "util/string.hpp"

namespace cmd { namespace rfc
{

void CWDCommand::DisplayFile(const fs::RealPath& path)
{
  try
  {
    if (util::path::Status(path.ToString()).IsReadable())
    {
      std::string lines;
      if (util::ReadFileToString(path.ToString(), lines))
      {
        control.PartReply(ftp::FileActionOkay, lines);
      }
    }
  }
  catch (const util::SystemError&)
  {
  }
}

void CWDCommand::ShowDiz(const fs::VirtualPath& path)
{
  for (const auto& diz : cfg::Get().ShowDiz())
  {
    if (diz.ACL().Evaluate(client.User().ACLInfo()))
    {
      DisplayFile(fs::MakeReal(path / diz.Path()));
    }
  }
}

void CWDCommand::MsgPath(const fs::VirtualPath& path)
{
  std::string slashPath = path.ToString();
  if (slashPath.back() != '/') slashPath += '/';
  
  for (const auto& mp : cfg::Get().Msgpath())
  {
    if (mp.ACL().Evaluate(client.User().ACLInfo()) &&
       util::WildcardMatch(mp.Path(), slashPath))
    {
      DisplayFile(fs::RealPath(mp.Filepath()));
    }
  }
}

void CWDCommand::ShowMessage(const fs::VirtualPath& path)
{
  MsgPath(path);
  ShowDiz(path);
}

void CWDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  util::Error e = fs::ChangeDirectory(client.User(),  path);
  if (e)
  {
    ShowMessage(fs::WorkDirectory());
    control.Reply(ftp::FileActionOkay, "CWD command successful."); 
    return;
  }
  
  fs::VirtualPath match;
  if (e.Errno() == ENOENT && 
     (e = fs::ChangeAlias(client.User(), fs::Path(argStr), match)))
  {
    ShowMessage(fs::WorkDirectory());
    control.Reply(ftp::FileActionOkay, "CWD command successful (Alias: " + 
          match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && 
      (e = fs::ChangeMatch(client.User(), path, match)))
  {
    ShowMessage(fs::WorkDirectory());
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;
  }

  if (e.Errno() == ENOENT && 
      (e = fs::ChangeCdpath(client.User(), fs::Path(argStr), match)))
  {
    ShowMessage(fs::WorkDirectory());
    control.Reply(ftp::FileActionOkay, "CWD command successful (Matched: " + 
                 match.ToString() + ").");
    return;    
  }

  control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  throw cmd::NoPostScriptError();
}

} /* rfc namespace */
} /* cmd namespace */
