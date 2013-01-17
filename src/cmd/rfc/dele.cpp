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

namespace cmd { namespace rfc
{

void DELECommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  bool loseCredits = 
    !acl::path::FileAllowed<acl::path::Nostats>(client.User(), path) && 
    fs::OwnerCache::Owner(fs::MakeReal(path)).UID() == client.User().UID();
  
  off_t size;
  util::Error e = fs::DeleteFile(client,  path, &size);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }

  if (loseCredits)
  {
    long long creditLoss = (size * client.UserProfile().Ratio()) / 1024;
    acl::UserCache::DecrCredits(client.User().Name(), creditLoss);
    db::stats::UploadDecr(client.User(), size / 1024);
    std::ostringstream os;
    os << "DELE command successful. (" << std::fixed << std::setprecision(2) 
       << creditLoss / 1024.0 << "MB credits lost)";
    control.Reply(ftp::FileActionOkay, os.str()); 
    throw cmd::NoPostScriptError();
  }

  control.Reply(ftp::FileActionOkay, "DELE command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
