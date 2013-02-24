#include <algorithm>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "util/string.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "cmd/site/change.hpp"
#include "acl/user.hpp"
#include "acl/misc.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "acl/util.hpp"
#include "db/user.hpp"
#include "logs/logs.hpp"
#include "acl/flags.hpp"
#include "fs/path.hpp"
#include "cmd/util.hpp"
#include "acl/group.hpp"

namespace cmd { namespace site
{

const std::vector<CHANGECommand::SettingDef> CHANGECommand::settings =
{
  { "ratio",          1,  "change|changegadmin",  &CHANGECommand::CheckRatio,
    "Non section specific ratio (0 is unlimited, -1 is default)"                        },
    
  { "sratio",         2,  "change",               &CHANGECommand::CheckSectionRatio,
    "Section specific ratio, <section> <ratio> (0 is unlimited)"                        },
    
  { "wkly_allotment", 2,  "change|changegadmin",  &CHANGECommand::CheckWeeklyAllotment,
    "Weekly allotment, optionally for specific section <allotment>[M|G] [<section>]"    },
    
  { "homedir",        1,  "changehomedir",        &CHANGECommand::CheckHomeDir,
    "Home directory"                                                                    },
    
  { "flags",          1,  "changeflags",          &CHANGECommand::CheckFlags,
    "Flags, prefixed with +|-|= to add/delete/set"                                      },
    
  { "idle_time",      1,  "change",               &CHANGECommand::CheckIdleTime,
    "Idle time (-1 site wide default, 0 is unlimited)"                                  },
    
  { "expires",        1,  "change",               &CHANGECommand::CheckExpires,
    "Expiration date in format YYYY-MM-DD or YYYY/MM/DD (never to disable)"             },
    
  { "num_logins",     1,  "change",               &CHANGECommand::CheckNumLogins,
    "Maximum number of simultaneous logins (-1 is unlimited, 0 to disallow)"            },
    
  { "tagline",        -1, "change",               &CHANGECommand::CheckTagline,
    "Tagline"                                                                           },
    
  { "comment",        -1, "change",               &CHANGECommand::CheckComment,
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
    if (ratio < -1) throw boost::bad_lexical_cast();
    
    if (ratio > cfg::Get().MaximumRatio())
    {
      control.Format(ftp::ActionNotOkay, "Ratio must be no larger than %1%.", cfg::Get().MaximumRatio());
      throw cmd::NoPostScriptError();
    }

    if (ratio == -1)
    {
      auto user = acl::User::Load("default");
      if (!user) 
      { 
        control.Format(ftp::ActionNotOkay, "Unable to load default user template.");
        throw cmd::NoPostScriptError();
      }
      ratio = user->DefaultRatio();
    }
    else
    if (gadmin && ratio > 0)
    {
      control.Format(ftp::ActionNotOkay, "Gadmin can only set ratio 0 (unlimited) or -1 (default).");
      throw cmd::NoPostScriptError();      
    }
    
    if (ratio == 0) display = "Unlimited";
    else display = "1:" + boost::lexical_cast<std::string>(ratio);

    return [ratio, gadmin, this](acl::User& user) -> bool 
            {
              if (gadmin && ratio == 0)
              {
                auto group = acl::Group::Load(user.PrimaryGID());
                if (!group)
                {
                  control.PartFormat(ftp::CommandOkay, "Unable to load group profile for %1%.", user.PrimaryGroup());
                  return false;
                }
                
                if (group->NumLeeches() >= group->LeechSlots())
                {
                  control.PartFormat(ftp::CommandOkay, "Maximum number of leech slots exceeded for group %1%.", 
                                     group->Name());
                  return false;
                }
              }
              
              user.SetDefaultRatio(ratio); 
              return true; 
            };
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
  
  std::string section = util::ToUpperCopy(args[3]);
  if (config.Sections().find(section) == config.Sections().end())
  {
    control.Format(ftp::ActionNotOkay, "Section %1% doesn't exist.", section);
    throw cmd::NoPostScriptError();
  }
  
  try
  {
    
    int ratio = boost::lexical_cast<int>(args[4]);
    if (ratio < 0) throw boost::bad_lexical_cast();
    
    if (ratio > cfg::Get().MaximumRatio())
    {
      control.Format(ftp::ActionNotOkay, "Ratio must be no larger than %1%.", cfg::Get().MaximumRatio());
      throw cmd::NoPostScriptError();
    }

    display = section + "(";
    if (ratio == 0) display += "Unlimited";
    else display += "1:" + boost::lexical_cast<std::string>(ratio);
    
    display += ")";
    
    return [ratio, section](acl::User& user) -> bool { user.SetSectionRatio(section, ratio); return true; };
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

  const cfg::Config& config = cfg::Get();
    
  std::string section;
  if (args.size() == 5)
  {
    section = util::ToUpperCopy(args[4]);
    if (config.Sections().find(section) == config.Sections().end())
    {
      control.Format(ftp::ActionNotOkay, "Section %1% doesn't exist.", section);
      throw cmd::NoPostScriptError();
    }
  }

  long long allotment;
  if (!ParseCredits(args[3], allotment)) throw cmd::SyntaxError();
  
  if (!section.empty()) display = section + "(";
  if (allotment == 0) display += "Disabled";
  else display += boost::lexical_cast<std::string>(allotment) + "KB";
  
  if (!section.empty()) display += ")";
  
  return [section, allotment, gadmin, this](acl::User& user) -> bool
          {
            if (gadmin && allotment > 0)
            {
              auto group = acl::Group::Load(user.PrimaryGID());
              if (!group)
              {
                control.PartFormat(ftp::CommandOkay, "Unable to load group profile for %1%.", user.PrimaryGroup());
                return false;
              }

              if (user.DefaultWeeklyAllotment() == 0 && group->NumAllotments() >= group->AllotmentSlots())
              {
                control.PartFormat(ftp::CommandOkay, "Maximum number of allotment slots exceeded for group %1%.", group->Name());
                return false;
              }

              if (group->TotalAllotmentSize() - user.DefaultWeeklyAllotment() + allotment > group->MaxAllotmentSize())
              {
                control.PartFormat(ftp::CommandOkay, "Maximum allotment size exceeded for group %1%.", group->Name());
                return false;
              }
            }
            
            user.SetSectionWeeklyAllotment(section, allotment); 
            return true; 
          };
}

CHANGECommand::SetFunction CHANGECommand::CheckHomeDir()
{
  std::string path = fs::PathFromUser(argStr.substr(args[1].length() + args[2].length() + 2)).ToString();  
  display = path;  
  return [path](acl::User& user) -> bool { user.SetHomeDir(path); return true; };
}

CHANGECommand::SetFunction CHANGECommand::CheckFlags()
{
  char action = args[3][0];
  
  std::string flags = util::ToUpperCopy(args[3].substr(1));
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
    case '+'  : return [flags](acl::User& user) -> bool { user.AddFlags(flags); return true; };
    case '-'  : return [flags](acl::User& user) -> bool
                        {
                          user.DelFlags(flags); 
                          // ensure the template flag is not deleted from default user
                          if (user.ID() == 1) user.AddFlag(acl::Flag::Template);
                          return true; 
                        };
    case '='  : return [flags](acl::User& user) -> bool
                        {
                          user.SetFlags(flags);
                          // ensure the template flag is not deleted from default user
                          if (user.ID() == 1) user.AddFlag(acl::Flag::Template);
                          return true;
                        };
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

    return [idleTime](acl::User& user) -> bool { user.SetIdleTime(idleTime); return true; };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckExpires()
{
  boost::optional<boost::gregorian::date> date;
  util::ToLower(args[3]);
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
  return [date](acl::User& user) -> bool { user.SetExpires(date); return true; };
}

CHANGECommand::SetFunction CHANGECommand::CheckNumLogins()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < -1) throw boost::bad_lexical_cast();
    
    if (logins == -1) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(logins);
    
    return [logins](acl::User& user) -> bool { user.SetNumLogins(logins); return true; };
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
  return [tagline](acl::User& user) -> bool { user.SetTagline(tagline); return true; };
}

CHANGECommand::SetFunction CHANGECommand::CheckComment()
{
  std::string comment = argStr.substr(args[1].length() + args[2].length() + 2);
  display = comment;
  return [comment](acl::User& user) -> bool {user.SetComment(comment); return true; };
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxUpSpeed()
{
  try
  {
    long long speed = boost::lexical_cast<long long>(args[3]);
    if (speed < 0) throw boost::bad_lexical_cast();
    if (speed == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(speed) + "KB/s";
    
    return [speed](acl::User& user) -> bool { user.SetMaxUpSpeed(speed); return true; };
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

    return [speed](acl::User& user) -> bool { user.SetMaxDownSpeed(speed); return true; };
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
    
    return [logins](acl::User& user) -> bool { user.SetMaxSimUp(logins); return true; };
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
    
    return [logins](acl::User& user) -> bool { user.SetMaxSimDown(logins); return true; };
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::Check()
{
  util::ToLower(args[2]);
  
  auto it = std::find_if(settings.begin(), settings.end(),
            [&](const SettingDef& def) -> bool { return def.name == args[2]; });
  if (it == settings.end()) throw cmd::SyntaxError();

  if (!acl::AllowSiteCmd(client.User(), it->aclKeyword)) throw cmd::PermissionError();
  if (it->maximumArgs != -1 && 
      static_cast<ssize_t>(args.size()) > it->maximumArgs + 3) throw cmd::SyntaxError();

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
  
  int changed = 0;
  for (auto uid : uids)
  {
    auto user = acl::User::Load(uid);
    if (user)
    {
      if (!set(*user)) break;
      ++changed;
      logs::Siteop(client.User().Name(), "changed '%1%' for '%2%' to '%3%'", args[2], user->Name(), display);
    }
  }

  assert(!display.empty());
  
  if (changed == 0)
    control.Format(ftp::CommandOkay, "No users changed.");
  else
  {  
    control.Format(ftp::CommandOkay, "Setting %1% changed for %2%: %3%", args[2], 
                   changed == 1 ? acl::UIDToName(uids[0]) : 
                   util::Format()("%i users", changed), display);
  }
}

}
}
