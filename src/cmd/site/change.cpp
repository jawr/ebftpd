#include <algorithm>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "cmd/site/change.hpp"
#include "acl/usercache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "acl/allowsitecmd.hpp"
#include "util/error.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

const std::vector<CHANGECommand::SettingDef> CHANGECommand::settings =
{
  { "ratio",          1,  "changeratio",    &CHANGECommand::CheckRatio            },
  { "sratio",         2,  "changeratio",    &CHANGECommand::CheckSectionRatio     },
  { "wkly_allotment", 2,  "changeallot",    &CHANGECommand::CheckWeeklyAllotment  },
  { "homedir",        1,  "changehomedir",  &CHANGECommand::CheckHomeDir          },
  { "flags",          1,  "changeflags",    &CHANGECommand::CheckFlags            },
  { "idle_time",      1,  "change",         &CHANGECommand::CheckIdleTime         },
  { "expires",        1,  "change",         &CHANGECommand::CheckExpires          },
  { "num_logins",     1,  "change",         &CHANGECommand::CheckNumLogins        },
  { "tagline",        1,  "change",         &CHANGECommand::CheckTagline          },
  { "comment",        1,  "change",         &CHANGECommand::CheckComment          },
  { "max_up_speed",   1,  "change",         &CHANGECommand::CheckMaxUpSpeed       },
  { "max_down_speed", 1,  "change",         &CHANGECommand::CheckMaxDownSpeed     },
  { "max_sim_up",     1,  "change",         &CHANGECommand::CheckMaxSimUp         },
  { "max_sim_down",   1,  "change",         &CHANGECommand::CheckMaxSimDown       }
};

CHANGECommand::SetFunction CHANGECommand::CheckRatio()
{
  try
  {
    int ratio = boost::lexical_cast<int>(args[3]);
    if (ratio < 0 || ratio > cfg::Get().MaximumRatio()) throw boost::bad_lexical_cast();

    if (ratio == 0) display = "Unlimited";
    else display = "1:" + boost::lexical_cast<std::string>(ratio);

    return boost::bind(&db::userprofile::SetRatio, _1, "", ratio);
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
  
  boost::to_upper(args[3]);
  if (config.Sections().find(args[3]) == config.Sections().end())
  {
    control.Reply(ftp::ActionNotOkay, "Section " + args[3] + " doesn't exist.");
    throw cmd::NoPostScriptError();
  }
  
  try
  {
    int ratio = boost::lexical_cast<int>(args[4]);
    if (ratio < 0 || ratio > cfg::Get().MaximumRatio()) throw boost::bad_lexical_cast();
    
    display = args[3] + "(";
    if (ratio == 0) display += "Unlimited";
    else display += "1:" + boost::lexical_cast<std::string>(ratio);
    
    display += ")";
    
    return boost::bind(&db::userprofile::SetRatio, _1, args[3], ratio);
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

CHANGECommand::SetFunction CHANGECommand::CheckWeeklyAllotment()
{
  const cfg::Config& config = cfg::Get();
  
  std::string section;
  if (args.size() == 5)
  {
    section = boost::to_upper_copy(args[4]);
    if (config.Sections().find(args[4]) == config.Sections().end())
    {
      control.Reply(ftp::ActionNotOkay, "Section " + args[4] + " doesn't exist.");
      throw cmd::NoPostScriptError();
    }
  }
  
  try
  {
    int allotment = boost::lexical_cast<int>(args[3]);
    if (allotment < 0) throw boost::bad_lexical_cast();
    
    if (!section.empty()) display = section + "(";
    if (allotment == 0) display += "Disabled";
    else display += boost::lexical_cast<std::string>(allotment) + "KB";
    
    if (!section.empty()) display += ")";
    
    return boost::bind(&db::userprofile::SetWeeklyAllotment, _1, allotment);
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
    control.Reply(ftp::ActionNotOkay, "Must be an absolute path.");
    throw cmd::NoPostScriptError();
  }
  
  display = path;  
  return boost::bind(&db::userprofile::SetHomeDir,  _1, path);
}

CHANGECommand::SetFunction CHANGECommand::CheckFlags()
{
  char action = args[3][0];
  
  std::string flags = boost::to_upper_copy(args[3].substr(1));
  if (flags.empty()) throw cmd::SyntaxError();
  
  if (!acl::ValidFlags(flags))
  {
    control.Reply(ftp::ActionNotOkay, "One or more invalid flags. See SITE FLAGS for a list.");
    throw cmd::NoPostScriptError();
  }
  
  if (flags.find(static_cast<char>(acl::Flag::Deleted)) != std::string::npos)
  {
    control.Reply(ftp::ActionNotOkay, "Flag 6 (deleted) cannot be changed with SITE CHANGE.");
    throw cmd::NoPostScriptError();
  }

  display = args[3];
  
  switch (action)
  {
    case '+'  : return boost::bind(&CHANGECommand::AddFlags, this, _1, flags);
    case '-'  : return boost::bind(&CHANGECommand::DelFlags, this, _1, flags);
    case '='  : return boost::bind(&CHANGECommand::SetFlags, this, _1, flags);
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
    return boost::bind(&db::userprofile::SetIdleTime, _1, idleTime);
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
      control.Reply(ftp::ActionNotOkay, "Date must be in format YYYY/MM/DD or YYYY-MM-DD.");
      throw cmd::NoPostScriptError();
    }
  }
  
  display = args[3];
  return boost::bind(&db::userprofile::SetExpires, _1, date);
}

CHANGECommand::SetFunction CHANGECommand::CheckNumLogins()
{
  try
  {
    int logins = boost::lexical_cast<int>(args[3]);
    if (logins < 0) throw boost::bad_lexical_cast();
    display = boost::lexical_cast<std::string>(logins);
    return boost::bind(&db::userprofile::SetNumLogins, _1, logins);
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
    control.Reply(ftp::ActionNotOkay, "Tagline contains invalid characters.");
    throw cmd::NoPostScriptError();
  }
  
  display = tagline;
  return boost::bind(&db::userprofile::SetTagline, _1, tagline);
}

CHANGECommand::SetFunction CHANGECommand::CheckComment()
{
  std::string comment = argStr.substr(args[1].length() + args[2].length() + 2);
  display = comment;
  return boost::bind(&db::userprofile::SetComment, _1, comment);
}

CHANGECommand::SetFunction CHANGECommand::CheckMaxUpSpeed()
{
  try
  {
    int speed = boost::lexical_cast<int>(args[3]);
    if (speed < 0) throw boost::bad_lexical_cast();
    if (speed == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(speed) + "KB/s";
    return boost::bind(&db::userprofile::SetMaxUpSpeed, _1, speed);
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
    int speed = boost::lexical_cast<int>(args[3]);
    if (speed < 0) throw boost::bad_lexical_cast();
    if (speed == 0) display = "Unlimited";
    else display = boost::lexical_cast<std::string>(speed) + "KB/s";
    return boost::bind(&db::userprofile::SetMaxDownSpeed, _1, speed);
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
    return boost::bind(&db::userprofile::SetMaxSimUp, _1, logins);
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
    return boost::bind(&db::userprofile::SetMaxSimDown, _1, logins);
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
}

void CHANGECommand::AddFlags(acl::UserID uid, const std::string& flags)
{
  acl::UserCache::AddFlags(acl::UserCache::UIDToName(uid), flags);
}

void CHANGECommand::DelFlags(acl::UserID uid, const std::string& flags)
{
  acl::UserCache::DelFlags(acl::UserCache::UIDToName(uid), flags);
}

void CHANGECommand::SetFlags(acl::UserID uid, const std::string& flags)
{
  acl::UserCache::SetFlags(acl::UserCache::UIDToName(uid), flags);
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
  
  auto uids = db::user::GetMultiUIDOnly(args[1]);  
  if (uids.empty())
  {
    control.Reply(ftp::ActionNotOkay, "No user's exist matching that criteria.");
    throw cmd::NoPostScriptError();
  }
  
  std::for_each(uids.begin(), uids.end(), set);

  assert(!display.empty());
  
  std::ostringstream os;
  os << "Setting " << args[2] << " changed for ";
  if (uids.size() == 1)
  {
     os << acl::UserCache::UIDToName(uids.front()) << ": ";
  }
  else
  {
    os << uids.size() << " users: ";
  }
  
  os << display;
  
  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
