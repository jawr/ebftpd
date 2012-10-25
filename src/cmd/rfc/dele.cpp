#include "cmd/rfc/dele.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stats.hpp"

namespace cmd { namespace rfc
{

cmd::Result DELECommand::Execute()
{
  off_t size = fs::SizeFile(client, argStr);
  util::Error e = fs::DeleteFile(client,  argStr);
  if (!e) control.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else 
  {
    control.Reply(ftp::FileActionOkay, "DELE command successful."); 
    long long bytes = (size * client.UserProfile().Ratio()) / 1000;
    acl::UserCache::DecrCredits(client.User().Name(), bytes);
    db::stats::UploadDecr(client.User(), bytes);
  }
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
