#include "cmd/rfc/mkd.hpp"
#include "fs/directory.hpp"
#include "acl/path.hpp"
#include "exec/check.hpp"
#include "cmd/error.hpp"
#include "db/index/index.hpp"
#include "db/dupe/dupe.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void MKDCommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(argStr));

  fs::Path messagePath;
  util::Error e(acl::path::Filter(client.User(), path.Basename(), messagePath));
  if (!e)
  {
    // should display above messagepath, we'll just reply for now
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
  
  if (acl::path::DirAllowed<acl::path::Indexed>(client.User(), path))
  {
    auto section = cfg::Get().SectionMatch(path.ToString());
    db::index::Add(path.ToString(), section ? section->Name() : "");
  }
  
  if (acl::path::DirAllowed<acl::path::Dupelog>(client.User(), path))
  {
    auto section = cfg::Get().SectionMatch(path.ToString());
    db::dupe::Add(path.Basename().ToString(), section ? section->Name() : "");    
  }
  
  control.Reply(ftp::PathCreated, "MKD command successful."); 
}

} /* rfc namespace */
} /* cmd namespace */
