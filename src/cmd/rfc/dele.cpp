#include <sstream>
#include <iomanip>
#include "cmd/rfc/dele.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stat.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/owner.hpp"
#include "logs/logs.hpp"
#include "cmd/error.hpp"
#include "stats/util.hpp"
#include "db/user/userprofile.hpp"

namespace cmd { namespace rfc
{

void DELECommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  bool loseCredits = fs::GetOwner(fs::MakeReal(path)).UID() == client.User().UID();
  
  off_t bytes;
  time_t modTime;
  util::Error e = fs::DeleteFile(client.User(),  path, &bytes, &modTime);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  auto section = cfg::Get().SectionMatch(path.ToString());
  bool nostats = !section || acl::path::FileAllowed<acl::path::Nostats>(client.User(), path);
  if (!nostats)
  {
    db::stats::UploadDecr(client.User(), bytes / 1024, modTime, section->Name());
  }

  if (loseCredits)
  {
    long long creditLoss = bytes / 1024 * stats::UploadRatio(client, path, section);
    if (creditLoss)
    {
      db::userprofile::DecrCredits(client.User().UID(), creditLoss, 
              section && section->SeparateCredits() ? section->Name() : "", true);
      std::ostringstream os;
      os << "DELE command successful. (" << std::fixed << std::setprecision(2) 
         << creditLoss / 1024.0 << "MB credits lost)";
      control.Reply(ftp::FileActionOkay, os.str()); 
      return;
    }
  }

  control.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
