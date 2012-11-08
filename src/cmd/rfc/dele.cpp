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

bool DELECommand::LosesCredits() const
{
  fs::Path absolute = (client.WorkDir() / argStr).Expand();
  fs::Path real = cfg::Get().Sitepath() + absolute;
  return !acl::path::FileAllowed<acl::path::Nostats>(client.User(), absolute) && 
          fs::OwnerCache::Owner(real).UID() == client.User().UID();
}

cmd::Result DELECommand::Execute()
{
  bool losesCredits = LosesCredits(); // must check before deleting file
  
  off_t size;
  util::Error e = fs::DeleteFile(client,  argStr, &size);
  if (!e) control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
  else 
  {
    if (losesCredits)
    {
      long long creditLoss = (size * client.UserProfile().Ratio()) / 1024;
      acl::UserCache::DecrCredits(client.User().Name(), creditLoss);
      db::stats::UploadDecr(client.User(), size / 1024);
      std::ostringstream os;
      os << "DELE command successful. (" << std::fixed << std::setprecision(2) 
         << creditLoss / 1024.0 << "MB credits lost)";
      control.Reply(ftp::FileActionOkay, os.str()); 
    }
    else
      control.Reply(ftp::FileActionOkay, "DELE command successful."); 
  }
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
