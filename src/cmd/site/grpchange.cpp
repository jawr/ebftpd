#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/grpchange.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
#include "util/error.hpp"
#include "db/group/group.hpp"
#include "db/group/groupprofile.hpp"
#include "cmd/error.hpp"
#include "acl/allowsitecmd.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

const std::vector<GRPCHANGECommand::SettingDef> GRPCHANGECommand::settings =
{
  { "slots",            1,  "grpchange",      &GRPCHANGECommand::CheckSlots         },
  { "leech_slots",      1,  "grpchange",      &GRPCHANGECommand::CheckLeechSlots    },
  { "allot_slots",      1,  "grpchange",      &GRPCHANGECommand::CheckAllotSlots    },
  { "max_allot_size",   1,  "grpchange",      &GRPCHANGECommand::CheckMaxAllotSize  },
  { "max_logins",       1,  "grpchange",      &GRPCHANGECommand::CheckMaxLogins     },
  { "description",      1,  "grpchange",      &GRPCHANGECommand::CheckDescription   },
  { "comment",          1,  "grpchange",      &GRPCHANGECommand::CheckComment       }
};

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckSlots()
{
  try
  {
    int slots = boost::lexical_cast<int>(args[3]);
    if (slots < -1) throw boost::bad_lexical_cast();
    if (slots == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(slots);
    return boost::bind(&db::groupprofile::SetSlots, _1, slots);
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
    return boost::bind(&db::groupprofile::SetLeechSlots, _1, slots);
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
    return boost::bind(&db::groupprofile::SetAllotSlots, _1, slots);
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
    return boost::bind(&db::groupprofile::SetMaxAllotSize, _1, allotment);
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
    return boost::bind(&db::groupprofile::SetMaxLogins, _1, logins);
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
  return boost::bind(&db::groupprofile::SetDescription, _1, description);
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::CheckComment()
{
  std::string comment = argStr.substr(args[1].length() + args[2].length() + 2);
  display = comment;
  return boost::bind(&db::groupprofile::SetComment, _1, comment);
}

GRPCHANGECommand::SetFunction GRPCHANGECommand::Check()
{
  boost::to_lower(args[2]);
  
  auto it = std::find_if(settings.begin(), settings.end(),
            [&](const SettingDef& def) { return def.name == args[2]; });
  if (it == settings.end()) throw cmd::SyntaxError();

  if (!acl::AllowSiteCmd(client.User(), it->aclKeyword)) throw cmd::PermissionError();
  if (static_cast<ssize_t>(args.size()) > it->maximumArgs + 3) throw cmd::SyntaxError();
  
  return it->check(this);
}

void GRPCHANGECommand::Execute()
{
  SetFunction set = Check();
  
  auto gids = db::group::GetMultiGIDOnly(args[1]);  
  if (gids.empty())
  {
    control.Format(ftp::ActionNotOkay, "No group's exist matching that criteria.");
    throw cmd::NoPostScriptError();
  }
  
  std::for_each(gids.begin(), gids.end(), set);
  assert(!display.empty());
  control.Format(ftp::CommandOkay, "Setting %1% changed for %2%: %3%", args[2], 
                 gids.size() == 1 ? acl::GroupCache::GIDToName(gids[0]) : 
                 util::Format()("%i groups", gids.size()), display);
}

}
}
