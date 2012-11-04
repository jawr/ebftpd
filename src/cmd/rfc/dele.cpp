#include "cmd/rfc/dele.hpp"
#include "fs/file.hpp"
#include "acl/usercache.hpp"
#include "db/stats/stat.hpp"

namespace cmd { namespace rfc
{

cmd::Result DELECommand::Execute()
{
  off_t size;
  util::Error e = fs::DeleteFile(client,  argStr, &size);
  if (!e) control.Reply(ftp::ActionNotOkay, "DELE failed: " + e.Message());
  else 
  {
    control.Reply(ftp::FileActionOkay, "DELE command successful."); 
    long long bytes = (size * client.UserProfile().Ratio()) / 1000;
    acl::UserCache::DecrCredits(client.User().Name(), bytes);
    db::stats::UploadDecr(client.User(), size/1000);
  }
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
