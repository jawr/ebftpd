#include <sstream>
#include <string>
#include <vector>
#include "util/string.hpp"
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/grpchange.hpp"
#include "acl/group.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "acl/misc.hpp"
#include "acl/util.hpp"
#include "db/group.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

const std::vector<GRPCHANGECommand::SettingDef> GRPCHANGECommand::settings =
{
  { "slots",            1,  "grpchange",      &GRPCHANGECommand::CheckSlots,
    "Number of slots (-1 is unlimited)"                                             },

  { "leech_slots",      1,  "grpchange",      &GRPCHANGECommand::CheckLeechSlots,
    "Number of leech slots (-1 is unlimited, -2 is disabled)"                       },
    
  { "allot_slots",      1,  "grpchange",      &GRPCHANGECommand::CheckAllotSlots,
    "Number of allotment slots (-1 is unlimited, -2 is disabled)"                   },
    
  { "max_allot_size",   1,  "grpchange",      &GRPCHANGECommand::CheckMaxAllotSize,
    "Maximum allotment size in kbytes (0 is unlimited)"                             },
    
  { "max_logins",       1,  "grpchange",      &GRPCHANGECommand::CheckMaxLogins,
    "Maximum simultaneous logins (-1 is unlimited, 0 is disallow)"                  },
    
  { "description",      -1, "grpchange",      &GRPCHANGECommand::CheckDescription,
    "Description"                                                                   },
    
  { "comment",          -1, "grpchange",      &GRPCHANGECommand::CheckComment,
    "Comment"                                                                       }
};

std::string GRPCHANGECommand::Syntax()
{
  std::ostringstream os;
  os << "Syntax: SITE GRPCHANGE <group> <setting> <value>\n"
        "        SITE GRPCHANGE {<group> [<group> ..]} <setting> <value>\n"
        "        SITE GRPCHANGE * <setting> <value>\n"
        "Settings:\n";

  std::string::size_type maxNameLen = 0;
  for (const auto& setting : settings)
    maxNameLen = std::max(maxNameLen, setting.name.length());

  for (const auto& setting : settings)
  {
    os << "          " << std::left << std::setw(maxNameLen) << setting.name 
       << ": " << setting.description << "\n";
  }
  return os.str();
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckSlots()
{
  try
  {
    int slots = boost::lexical_cast<int>(args[3]);
    if (slots < -1) throw boost::bad_lexical_cast();
    if (slots == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(slots);
    
    return [slots](acl::Group& group) { group.SetSlots(slots); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckLeechSlots()
{
  try
  {
    int slots = boost::lexical_cast<int>(args[3]);
    if (slots < -2) throw boost::bad_lexical_cast();
    if (slots == -1) display = "Unlimited";
    else if (slots == -2) display = "Disabled";
    else display = boost::lexical_cast<std::string>(slots);
    
    return [slots](acl::Group& group) { group.SetLeechSlots(slots); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckAllotSlots()
{
  try
  {
    int slots = boost::lexical_cast<int>(args[3]);
    if (slots < -2) throw boost::bad_lexical_cast();
    if (slots == -1) display = "Unlimited";
    else if (slots == -2) display = "Disabled";
    else display = boost::lexical_cast<std::string>(slots);
    
    return [slots](acl::Group& group) { group.SetAllotmentSlots(slots); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckMaxAllotSize()
{
  try
  {
    long long allotment = boost::lexical_cast<long long>(args[3]);
    if (allotment < 0) throw boost::bad_lexical_cast();
    if (allotment == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(allotment);
    
    return [allotment](acl::Group& group) { group.SetMaxAllotmentSize(allotment); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckMaxLogins()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < -1) throw boost::bad_lexical_cast();
    if (logins == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(logins);
    
    return [logins](acl::Group& group) { group.SetMaxLogins(logins); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckDescription()
{
  std::string description = argStr.substr(args[1].length() + args[2].length() + 2);
  if (!acl::Validate(acl::ValidationType::Tagline, description))
  {
    control.Format(ftp::ActionNotOkay, "Description contains invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  display = description;
  
  return [description](acl::Group& group) { group.SetDescription(description); };
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckComment()
{
  std::string comment = argStr.substr(args[1].length() + args[2].length() + 2);
  display = comment;
  return [comment](acl::Group& group) { group.SetComment(comment); };
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::Check()
{
  util::ToLower(args[2]);
  
  auto it = std::find_if(settings.begin(), settings.end(),
            [&](const SettingDef& def) { return def.name == args[2]; });
  if (it == settings.end()) throw cmd::SyntaxError();

  if (!acl::AllowSiteCmd(client.User(), it->aclKeyword)) throw cmd::PermissionError();
  if (it->maximumArgs != -1 &&
      static_cast<ssize_t>(args.size()) > it->maximumArgs + 3) throw cmd::SyntaxError();
  
  return it->check(this);
}

void GRPCHANGECommand::Execute()
{
  SetFunction set = Check();
  
  auto gids = acl::Group::GetGIDs(args[1]);
  if (gids.empty())
  {
    control.Format(ftp::ActionNotOkay, "No group's exist matching that criteria.");
    throw cmd::NoPostScriptError();
  }
  
  for (auto gid : gids)
  {
    auto group = acl::Group::Load(gid);
    if (group)
    {
      set(*group);
      logs::Siteop(client.User().Name(), "GROUP_" + args[2], group->Name(), display);
    }
  }

  assert(!display.empty());
  control.Format(ftp::CommandOkay, "Setting %1% changed for %2%: %3%", args[2], 
                 gids.size() == 1 ? acl::GIDToName(gids[0]) : 
                 util::Format()("%i groups", gids.size()), display);
}

}
}
