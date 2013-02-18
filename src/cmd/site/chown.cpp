#include <sstream>
#include <vector>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/chown.hpp"
#include "fs/globiterator.hpp"
#include "fs/dircontainer.hpp"
#include "cmd/error.hpp"
#include "util/path/status.hpp"
#include "util/enumbitwise.hpp"
#include "util/string.hpp"
#include "acl/group.hpp"

namespace cmd { namespace site
{

void CHOWNCommand::Process(fs::VirtualPath pathmask)
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
        fs::SetOwner(fs::MakeReal(entryPath), owner);
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
        "CHOWN " + pathmask.ToString() + ": " + e.Message());
  }
}

void CHOWNCommand::ParseArgs()
{
  int n = 1;
  if (boost::to_lower_copy(args[1]) == "-r") 
  {
    ++n;
    recursive = true;
  }
  
  std::vector<std::string> owners;
  boost::split(owners, args[n], boost::is_any_of(":"));
  if (owners.empty() || owners.size() > 2) throw cmd::SyntaxError();
  user = owners[0];
  if (owners.size() == 2) group = owners[1];
  
  std::string::size_type pos =
      util::string::FindNthNonConsecutiveChar(argStr, ' ', n);
  if (pos == std::string::npos) throw cmd::SyntaxError();
  
  patharg = argStr.substr(pos);
  boost::trim(patharg);
}

void CHOWNCommand::Execute()
{
  ParseArgs();

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
    uid = acl::NameToUID(user);
    if (uid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "User " + user + " doesn't exist.");
      return;
    }
  }
  
  acl::GroupID gid = -1;
  if (!group.empty())
  {
    gid = acl::NameToGID(group);
    if (gid == -1)
    {
      control.Reply(ftp::ActionNotOkay, "Group " + group + " doesn't exist.");
      return;
    }
  }
  
  owner = fs::Owner(uid, gid);

  Process(fs::PathFromUser(patharg));
  
  std::ostringstream os;
  os << "CHOWN finished (okay on: "
     << dirs << " directories, " << files 
     << " files / failures: " << failed << ").";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
