#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "cmd/site/user.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "db/user/userprofile.hpp"
#include "acl/userprofile.hpp"
#include "acl/groupcache.hpp"
#include "db/user/userprofile.hpp"
#include "db/group/group.hpp"
#include "util/error.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"
#include "cfg/get.hpp"
#include "acl/util.hpp"

namespace cmd { namespace site
{

void USERCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() && 
      !acl::AllowSiteCmd(client.User(), "user"))
  {
    throw cmd::PermissionError();
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("user"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::string userName = args.size() == 2 ? args[1] : client.User().Name();
  acl::User user;
  try
  {
    user = acl::UserCache::User(userName);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  acl::UserProfile profile;
  try
  {
    profile = db::userprofile::Get(user.UID());
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  std::string creator = "<ebftpd>";
  try
  {
    acl::User creatorUser = acl::UserCache::User(profile.Creator());
    creator = creatorUser.Name();
  }
  catch (const util::RuntimeError& e)
  {
    if (profile.Creator() != 0) creator = "<deleted>";
  }

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  body.RegisterValue("user", user.Name());
  body.RegisterValue("logged_in", profile.LoggedIn());
  body.RegisterValue("created", boost::gregorian::to_simple_string(profile.Created()));

  if (profile.LastLogin())
  {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff = now - *profile.LastLogin();
    body.RegisterValue("last_login", boost::posix_time::to_simple_string(*profile.LastLogin()));
    body.RegisterValue("since_login", boost::posix_time::to_simple_string(diff));
  }
  else
    body.RegisterValue("last_login", "Never");

  if (profile.Expires())
    body.RegisterValue("expires", boost::gregorian::to_simple_string(*profile.Expires()));
  else
    body.RegisterValue("expires", "Never");
  body.RegisterValue("creator", creator);
  body.RegisterValue("flags", user.Flags());
  body.RegisterValue("ratio", acl::RatioString(profile));  
  body.RegisterValue("credits", acl::CreditString(profile));
  body.RegisterValue("primary_group", 
    acl::GroupCache::GIDToName(user.PrimaryGID()));

  std::ostringstream secondaryGroups;
  auto secondary = user.SecondaryGIDs();
  if (secondary.size() > 0)
  {
    acl::Group group;
    for (auto& gid: user.SecondaryGIDs())
    {
      try
      {
        group = acl::GroupCache::Group(gid);
        secondaryGroups << group.Name() << " ";
      }
      catch (const util::RuntimeError& e)
      {
        secondaryGroups << "Error: " << e.Message() << " ";
      }
    }
  }
  
  body.RegisterValue("secondary_groups", secondaryGroups.str());
  body.RegisterValue("tagline", profile.Tagline());
  body.RegisterValue("comment", profile.Comment());
  body.RegisterSize("weekly_allot", profile.WeeklyAllotment());

  std::vector<std::string> masks;
  auto ok = acl::UserCache::ListIPMasks(user.Name(), masks);
  int idx = 0;
  for (auto& i : masks)
  {
    std::ostringstream maskIdx;
    maskIdx << "ip" << idx++;
    body.RegisterValue(maskIdx.str(), i);
  }

  std::ostringstream os;
  os << head.Compile();
  os << body.Compile();
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}

