//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sstream>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/grpchange.hpp"
#include "acl/group.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "acl/misc.hpp"
#include "acl/util.hpp"
#include "db/group/group.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"
#include "util/string.hpp"

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
    int slots = util::StrToInt(args[3]);
    if (slots < -1) throw std::bad_cast();
    if (slots == -1) display = "Unlimited";
    else display = std::to_string(slots);
    
    return [slots](acl::Group& group) { group.SetSlots(slots); };
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckLeechSlots()
{
  try
  {
    int slots = util::StrToInt(args[3]);
    if (slots < -2) throw std::bad_cast();
    if (slots == -1) display = "Unlimited";
    else if (slots == -2) display = "Disabled";
    else display = std::to_string(slots);
    
    return [slots](acl::Group& group) { group.SetLeechSlots(slots); };
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckAllotSlots()
{
  try
  {
    int slots = util::StrToInt(args[3]);
    if (slots < -2) throw std::bad_cast();
    if (slots == -1) display = "Unlimited";
    else if (slots == -2) display = "Disabled";
    else display = std::to_string(slots);
    
    return [slots](acl::Group& group) { group.SetAllotmentSlots(slots); };
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckMaxAllotSize()
{
  try
  {
    long long allotment = util::StrToLLong(args[3]);
    if (allotment < 0) throw std::bad_cast();
    if (allotment == -1) display = "Unlimited";
    else display = std::to_string(allotment);
    
    return [allotment](acl::Group& group) { group.SetMaxAllotmentSize(allotment); };
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckMaxLogins()
{
  try
  {
    int logins = util::StrToInt(args[3]);
    if (logins < -1) throw std::bad_cast();
    if (logins == -1) display = "Unlimited";
    else display = std::to_string(logins);
    
    return [logins](acl::Group& group) { group.SetMaxLogins(logins); };
  }
  catch (const std::bad_cast&)
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
      logs::Siteop(client.User().Name(), "changed '%1%' for '%2%' to '%3%'", args[2], group->Name(), display);
    }
  }

  assert(!display.empty());
  control.Format(ftp::CommandOkay, "Setting %1% changed for %2%: %3%", args[2], 
                 gids.size() == 1 ? acl::GIDToName(gids[0]) : 
                 util::Format()("%i groups", gids.size()), display);
}

}
}
