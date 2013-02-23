#include <sstream>
#include <string>
#include "cmd/site/chmod.hpp"
#include "fs/chmod.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cmd/error.hpp"
#include "logs/logs.hpp"
#include "util/path/status.hpp"
#include "util/enumbitwise.hpp"

namespace cmd { namespace site
{

void CHMODCommand::Process(fs::VirtualPath pathmask)
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
        util::Error e = fs::Chmod(client.User(), entryPath, *mode);
        if (!e)
        {
          ++failed;
          control.PartReply(ftp::CommandOkay, "CHOWN " +
              entryPath.ToString() + ": " + e.Message());
        }
        else
        if (status.IsDirectory()) ++dirs;
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
  util::ToLower(args[1]);
  if (args[1] == "-r")
  {
    ++n;
    recursive = true;
    util::ToLower(args[n]);
  }

  modeStr = args[n];

  std::string::size_type pos =
      util::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) throw cmd::SyntaxError();

  patharg = argStr.substr(pos);
  util::Trim(patharg);
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
