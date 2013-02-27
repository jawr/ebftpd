#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"
#include "acl/path.hpp"
#include "exec/check.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "db/dupe/dupe.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "fs/path.hpp"
#include "acl/user.hpp"

namespace cmd { namespace rfc
{

void MKDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  util::Error e(acl::path::Filter(client.User(), path.Basename()));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "Directory name contains one or more invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  if (!exec::PreDirCheck(client, path)) throw cmd::NoPostScriptError();

  e = fs::CreateDirectory(client.User(),  path);
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    throw cmd::NoPostScriptError();
  }
  
  const cfg::Config& config = cfg::Get();
  
  if (config.IsIndexed(path.ToString()))
  {
    auto section = config.SectionMatch(path.ToString());
    db::index::Add(path.ToString(), section ? section->Name() : "");
  }
  
  if (config.IsDupeLogged(path.ToString()))
  {
    auto section = config.SectionMatch(path.ToString());
    db::dupe::Add(path.Basename().ToString(), section ? section->Name() : "");    
  }
  
  if (config.IsEventLogged(path.ToString()))
  {
    logs::Event("NEWDIR", "path", fs::MakeReal(path).ToString(),
                "user", client.User().Name(), 
                "group", client.User().PrimaryGroup(), 
                "tagline", client.User().Tagline());
  }
  
  control.Reply(ftp::PathCreated, "MKD command successful.");  
}

} /* rfc namespace */
} /* cmd namespace */
