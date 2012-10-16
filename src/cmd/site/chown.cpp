#include <sstream>
#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/chown.hpp"
#include "fs/dircontainer.hpp"
#include "util/string.hpp"
#include "cfg/get.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"

namespace cmd { namespace site
{

void CHOWNCommand::Process(const fs::Path& pathmask)
{
  using util::string::WildcardMatch;
  const cfg::Config& config = cfg::Get();
  fs::Path dirname = pathmask.Dirname();
  fs::Path absolute = (client.WorkDir() / pathmask).Expand();
  try
  {
    fs::DirContainer dir(client, absolute.Dirname());
    for (auto& entry : dir)
    {
      if (!WildcardMatch(absolute.Basename(), entry))
        continue;

      fs::Path fullPath = (absolute.Dirname() / entry).Expand();
      fs::Path relative = (dirname / entry).Expand();
      try
      {
        fs::Status status(config.Sitepath() + fullPath);
        fs::OwnerCache::Chown(fullPath, owner);
        if (status.IsDirectory())
        {
          ++dirs;
          if (recursive && !status.IsSymLink()) 
            Process((relative / "*").Expand());
        }
        else ++files;
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "CHOWN " + 
            relative.ToString() + ": " + e.Message());        
      }
    }
  }
  catch (const util::SystemError& e)
  {
    ++failed;
    control.PartReply(ftp::CommandOkay, 
        "CHOWN " + pathmask.ToString() + ": " + e.Message());
  }
}

bool CHOWNCommand::ParseArgs()
{
  int n = 1;
  boost::to_lower(args[1]);
  if (args[1] == "-r") 
  {
    ++n;
    recursive = true;
    boost::to_lower(args[n]);
  }
  
  std::vector<std::string> owners;
  boost::split(owners, args[n], boost::is_any_of(":"));
  if (owners.empty() || owners.size() > 2) return false;
  user = owners[0];
  if (owners.size() == 2) group = owners[1];
  
  std::string::size_type pos =
      util::string::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) return false;
  
  pathmaskStr = argStr.substr(pos);
  boost::trim(pathmaskStr);
  return true;
}

// SITE CHOWN [-R] <MODE> <PATHMASK.. ..>
cmd::Result CHOWNCommand::Execute()
{
  if (!ParseArgs()) return cmd::Result::SyntaxError;

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chown!");
    return cmd::Result::Okay;
  }
  
  acl::UserID uid = -1;
  if (!user.empty())
  {
    uid = acl::UserCache::NameToUID(user);
    if (uid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "CHMOD failed: User doesn't exist");
      return cmd::Result::Okay;
    }
  }
  
  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::GroupCache::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "CHMOD failed: Group doesn't exist");
      return cmd::Result::Okay;
    }
  }
  
  owner = fs::Owner(uid, gid);

  Process(pathmaskStr);
  
  std::ostringstream os;
  os << "CHOWN finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
