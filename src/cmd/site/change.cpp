#include <algorithm>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "cmd/site/change.hpp"
#include "acl/user.hpp"
#include "acl/misc.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "acl/util.hpp"
#include "db/user.hpp"

namespace cmd { namespace site
{

const std::vector<CHANGECommand::SettingDef> CHANGECommand::settings =
{
  { "ratio",          1,  "change|changegadmin",  &CHANGECommand::CheckRatio,
    "Non section specific ratio (0 is unlimited)"                                       },
    
  { "sratio",         2,  "change",               &CHANGECommand::CheckSectionRatio,
    "Section specific ratio, <section> <ratio> (0 is unlimited)"                        },
    
  { "wkly_allotment", 2,  "change|changegadmin",  &CHANGECommand::CheckWeeklyAllotment,
    "Weekly allotment, optionally for specific section <allotment> [<section>]"         },
    
  { "homedir",        1,  "changehomedir",        &CHANGECommand::CheckHomeDir,
    "Home directory"                                                                    },
    
  { "flags",          1,  "changeflags",          &CHANGECommand::HasFlags,
    "Flags, prefixed with +|-|= to add/delete/set"                                      },
    
  { "idle_time",      1,  "change",               &CHANGECommand::CheckIdleTime,
    "Idle time (-1 is disabled, 0 is unlimited)"                                        },
    
  { "expires",        1,  "change",               &CHANGECommand::CheckExpires,
    "Expiration date in format YYYY-MM-DD or YYYY/MM/DD (never to disable)"             },
    
  { "num_logins",     1,  "change",               &CHANGECommand::CheckNumLogins,
    "Maximum number of simultaneous logins"                                             },
    
  { "tagline",        1,  "change",               &CHANGECommand::CheckTagline,
    "Tagline"                                                                           },
    
  { "comment",        1,  "change",               &CHANGECommand::CheckComment,
    "Comment"                                                                           },
    
  { "max_up_speed",   1,  "change",               &CHANGECommand::CheckMaxUpSpeed,
    "Maximum upload speed in kbyte/s (0 is unlimited)"                                  },
    
  { "max_down_speed", 1,  "change",               &CHANGECommand::CheckMaxDownSpeed,
    "Maximum download speed in kbyte/s (0 is unlimited)"                                },
    
  { "max_sim_up",     1,  "change",               &CHANGECommand::CheckMaxSimUp,
    "Maximum simultaneous uploads (-1 is unlimited, 0 to disallow)"                     },
    
  { "max_sim_down",   1,  "change",               &CHANGECommand::CheckMaxSimDown,
    "Maximum simultaneous downloads (-1 is unlimited, 0 to disallow)"                   }
};

std::string CHANGECommand::Syntax()
{
  std::ostringstream os;
  os << "Syntax: SITE CHANGE <user> <setting> <value>\n"
        "        SITE CHANGE {<user> [<user> ..]} <setting> <value>\n"
        "        SITE CHANGE * <setting> <value>\n"
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

CHANGECommand::SetFunction CHANGECommand::CheckRatio()
{
  gadmin = !acl::AllowSiteCmd(client.User(), "change") &&
           acl::AllowSiteCmd(client.User(), "changegadmin");
  try
  {
    int ratio = boost::lexical_cast<int>(args[3]);
    if (ratio < 0 || ratio > cfg::Get().MaximumRatio()) throw boost::bad_lexical_cast();

    if (ratio == 0) display = "Unlimited";
    else display = "1:" + boost::lexical_cast<std::string>(ratio);

    return [ratio](acl::User& user) { user.SetDefaultRatio(ratio); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckSectionRatio()
{
  if (args.size() < 4) throw cmd::SyntaxError();
  
  const cfg::Config& config = cfg::Get();
  
  std::string section = boost::to_upper_copy(args[3]);
  if (config.Sections().find(section) == config.Sections().end())
  {
    control.Format(ftp::ActionNotOkay, "Section %1% doesn't exist.", section);
    throw cmd::NoPostScriptError();
  }
  
  try
  {
    int ratio = boost::lexical_cast<int>(args[4]);
    if (ratio < 0 || ratio > cfg::Get().MaximumRatio()) throw boost::bad_lexical_cast();
    
    display = section + "(";
    if (ratio == 0) display += "Unlimited";
    else display += "1:" + boost::lexical_cast<std::string>(ratio);
    
    display += ")";
    
    return [ratio, section](acl::User& user) { user.SetSectionRatio(section, ratio); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckWeeklyAllotment()
{
  gadmin = !acl::AllowSiteCmd(client.User(), "change") &&
           acl::AllowSiteCmd(client.User(), "changegadmin");

  control.Format(ftp::NotImplemented, "Not finished");
  throw cmd::NoPostScriptError();

  const cfg::Config& config = cfg::Get();
    
  std::string section;
  if (args.size() == 5)
  {
    section = boost::to_upper_copy(args[4]);
    if (config.Sections().find(args[4]) == config.Sections().end())
    {
      control.Format(ftp::ActionNotOkay, "Section %1% doesn't exist.", args[4]);
      throw cmd::NoPostScriptError();
    }
  }
  
  try
  {
    long long allotment = boost::lexical_cast<long long>(args[3]);
    if (allotment < 0) throw boost::bad_lexical_cast();
    
    if (!section.empty()) display = section + "(";
    if (allotment == 0) display += "Disabled";
    else display += boost::lexical_cast<std::string>(allotment) + "KB";
    
    if (!section.empty()) display += ")";
    
    return [allotment](acl::User& user) { user.SetWeeklyAllotment(allotment); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckHomeDir()
{
  std::string path = argStr.substr(args[1].length() + args[2].length() + 2);
  
  assert(!path.empty());
  if (path[0] != '/')
  {
    control.Format(ftp::ActionNotOkay, "Must be an absolute path.");
    throw cmd::NoPostScriptError();
  }
  
  display = path;  
  return [path](acl::User& user) { user.SetHomeDir(path); };
}

CHANGECommand::SetFunction CHANGECommand::HasFlags()
{
  char action = args[3][0];
  
  std::string flags = boost::to_upper_copy(args[3].substr(1));
  if (flags.empty()) throw cmd::SyntaxError();
  
  if (!acl::ValidFlags(flags))
  {
    control.Format(ftp::ActionNotOkay, "One or more invalid flags. See SITE FLAGS for a list.");
    throw cmd::NoPostScriptError();
  }
  
  if (flags.find(static_cast<char>(acl::Flag::Deleted)) != std::string::npos)
  {
    control.Format(ftp::ActionNotOkay, "Flag 6 (deleted) cannot be changed with SITE CHANGE.");
    throw cmd::NoPostScriptError();
  }

  if (flags.find(static_cast<char>(acl::Flag::Gadmin)) != std::string::npos)
  {
    control.Format(ftp::ActionNotOkay, "Flag 2 (gadmin) cannot be changed with SITE CHANGE.");
    throw cmd::NoPostScriptError();
  }

  display = args[3];
  
  switch (action)
  {
    case '+'  : return [flags](acl::User& user) { user.AddFlags(flags); };
    case '-'  : return [flags](acl::User& user) { user.DelFlags(flags); };
    case '='  : return [flags](acl::User& user) { user.SetFlags(flags); };
    default   : throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckIdleTime()
{
  try
  {
    int idleTime = boost::lexical_cast<int>(args[3]);
    if (idleTime < -1) throw boost::bad_lexical_cast();
    if (idleTime == -1) display = "Unset";
    else if (idleTime == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(idleTime) + " seconds";

    return [idleTime](acl::User& user) { user.SetIdleTime(idleTime); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckExpires()
{
  boost::optional<boost::gregorian::date> date;
  boost::to_lower(args[3]);
  if (args[3] != "never")
  {
    try
    {
      boost::replace_all(args[3], "/", "-");
      try
      {
        date.reset(boost::gregorian::from_simple_string(args[3]));
      }
      catch (const std::out_of_range&)
      {
        throw boost::bad_lexical_cast();
      }
    }
    catch (const boost::bad_lexical_cast&)
    {
      control.Format(ftp::ActionNotOkay, "Date must be in format YYYY/MM/DD or YYYY-MM-DD.");
      throw cmd::NoPostScriptError();
    }
  }
  
  display = args[3];
  return [date](acl::User& user) { user.SetExpires(date); };
}

CHANGECommand::SetFunction CHANGECommand::CheckNumLogins()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < 0) throw boost::bad_lexical_cast();
    display = boost::lexical_cast<std::string>(logins);
    
    return [logins](acl::User& user) { user.SetNumLogins(logins); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckTagline()
{
  std::string tagline = argStr.substr(args[1].length() + args[2].length() + 2);
  if (!acl::Validate(acl::ValidationType::Tagline, tagline))
  {
    control.Format(ftp::ActionNotOkay, "Tagline contains invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  display = tagline;
  return [tagline](acl::User& user) { user.SetTagline(tagline); };
}

CHANGECommand::SetFunction CHANGECommand::CheckComment()
{
  std::string comment = argStr.substr(args[1].length() + args[2].length() + 2);
  display = comment;
  return [comment](acl::User& user) {user.SetComment(comment); };
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxUpSpeed()
{
  try
  {
    long long speed = boost::lexical_cast<long long>(args[3]);
    if (speed < 0) throw boost::bad_lexical_cast();
    if (speed == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(speed) + "KB/s";
    
    return [speed](acl::User& user) { user.SetMaxUpSpeed(speed); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxDownSpeed()
{
  try
  {
    long long speed = boost::lexical_cast<long long>(args[3]);
    if (speed < 0) throw boost::bad_lexical_cast();
    if (speed == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(speed) + "KB/s";

    return [speed](acl::User& user) { user.SetMaxDownSpeed(speed); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxSimUp()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < -1) throw boost::bad_lexical_cast();
    if (logins == 0) display = "Disabled";
    else if (logins == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(logins);
    
    return [logins](acl::User& user) { user.SetMaxSimUp(logins); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxSimDown()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < -1) throw boost::bad_lexical_cast();
    if (logins == 0) display = "Disabled";
    else if (logins == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(logins);
    
    return [logins](acl::User& user) { user.SetMaxSimDown(logins); };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::Check()
{
  boost::to_lower(args[2]);
  
  auto it = std::find_if(settings.begin(), settings.end(),
            [&](const SettingDef& def) { return def.name == args[2]; });
  if (it == settings.end()) throw cmd::SyntaxError();

  if (!acl::AllowSiteCmd(client.User(), it->aclKeyword)) throw cmd::PermissionError();
  if (static_cast<ssize_t>(args.size()) > it->maximumArgs + 3) throw cmd::SyntaxError();
  
  return it->check(this);
}

void CHANGECommand::Execute()
{
  SetFunction set = Check();
  
  auto uids = acl::User::GetUIDs(args[1]);  
  if (uids.empty())
  {
    control.Format(ftp::ActionNotOkay, "No user's exist matching that criteria.");
    throw cmd::NoPostScriptError();
  }
  
  if (gadmin)
  {
    auto it = std::find_if(uids.begin(), uids.end(), 
                [&](acl::UserID uid)
                {
                  return !client.User().HasGadminGID(acl::UIDToPrimaryGID(uid));
                });
    if (it != uids.end()) throw cmd::PermissionError();
  }
  
  for (auto uid : uids)
  {
    auto user = acl::User::Load(uid);
    if (user) set(*user);
  }

  assert(!display.empty());
  control.Format(ftp::CommandOkay, "Setting %1% changed for %2%: %3%", args[2], 
                 uids.size() == 1 ? acl::UIDToName(uids[0]) : 
                 util::Format()("%i users", uids.size()), display);
}

}
}
