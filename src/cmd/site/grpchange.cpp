#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/grpchange.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"
#include "db/group/group.hpp"
#include "db/group/groupprofile.hpp"


namespace cmd { namespace site
{

cmd::Result GRPCHANGECommand::Execute()
{
  std::string setting = args[2];
  std::string value = args[3];

  boost::to_lower(setting);

  util::Error ok;
  std::ostringstream os;

  std::vector<acl::Group> groups;

  if (args[1] == "*")
    groups = db::group::GetAll();
  else
  {
    std::vector<std::string> groupToks;
    boost::trim(args[1]);
    boost::split(groupToks, args[1], boost::is_any_of("\t "), 
      boost::token_compress_on);

    if (groupToks.empty()) return cmd::Result::SyntaxError;
  
    for (auto& group: groupToks)
    {
      try
      {
        groups.emplace_back(acl::GroupCache::Group(group));
      }
      catch (const util::RuntimeError& e)
      {
        os << "Error: " << e.Message() << "\n";
      }
    }
  } 

  int i = 0;
  if (groups.size() > 1)
  {
    os << "Updating (" << groups.size() << ") groups:";
    ++i;
  }

  for (auto& group: groups)
  {
    if (i++ > 0) os << "\n";

    if (setting == "slots")
      ok = db::groupprofile::SetSlots(group.GID(), value);

    else if (setting == "leech_slots")
      ok = db::groupprofile::SetLeechSlots(group.GID(), value);

    else if (setting == "allot_slots")
      ok = db::groupprofile::SetAllotSlots(group.GID(), value);

    else if (setting == "max_allot_slots")
      ok = db::groupprofile::SetMaxAllotSlots(group.GID(), value);

    else if (setting == "max_logins")
      ok = db::groupprofile::SetMaxLogins(group.GID(), value);

    else
    {
      control.Reply(ftp::ActionNotOkay, "Error: " + setting + " field not found.");
      return cmd::Result::Okay;
    }

    if (!ok)
      os << "Error: " << ok.Message();
    else 
      os << "Updated " << group.Name() << " " << setting << " to: " << value;
  } 

  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

// end
}
}
