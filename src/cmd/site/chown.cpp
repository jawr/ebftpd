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
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void CHOWNCommand::Process(const fs::Path& pathmask)
{
  using util::string::WildcardMatch;
  const cfg::Config& config = cfg::Get();
  try
  {
    fs::DirContainer dir(client, pathmask.Dirname());
    for (auto& entry : dir)
    {
      if (!WildcardMatch(pathmask.Basename(), entry))
        continue;

      fs::Path fullPath = (pathmask.Dirname() / entry).Expand();
      fs::Path real = config.Sitepath() + fullPath;
      try
      {
        fs::Status status(real);
        fs::OwnerCache::Chown(real, owner);
        if (status.IsDirectory())
        {
          ++dirs;
          if (recursive && !status.IsSymLink()) 
            Process((fullPath / "*").Expand());
        }
        else ++files;
      }
      catch (const util::SystemError& e)
      {
        ++failed;
        control.PartReply(ftp::CommandOkay, "CHOWN " + 
            fullPath.ToString() + ": " + e.Message());        
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
  if (boost::to_lower_copy(args[1]) == "-r") 
  {
    ++n;
    recursive = true;
  }
  
  std::vector<std::string> owners;
  boost::split(owners, args[n], boost::is_any_of(":"));
  if (owners.empty() || owners.size() > 2) return false;
  user = owners[0];
  if (owners.size() == 2) group = owners[1];
  
  std::string::size_type pos =
      util::string::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) return false;
  
  pathmask = argStr.substr(pos);
  boost::trim(pathmask);
  return true;
}

void CHOWNCommand::Execute()
{
  if (!ParseArgs()) throw cmd::SyntaxError();

  if (recursive && !client.ConfirmCommand(argStr))
  {
    control.Reply(ftp::CommandOkay, 
        "Repeat the command to confirm you "
        "want to do recursive chown!");
    return;
  }
  
  acl::UserID uid = -1;
  if (!user.empty())
  {
    uid = acl::UserCache::NameToUID(user);
    if (uid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "User " + user + " doesn't exist.");
      return;
    }
  }
  
  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::GroupCache::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + group + " doesn't exist.");
      return;
    }
  }
  
  owner = fs::Owner(uid, gid);

  Process((client.WorkDir() / pathmask).Expand());
  
  std::ostringstream os;
  os << "CHOWN finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
