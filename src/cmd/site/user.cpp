#include <sstream>
#include "util/string.hpp"
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "cmd/site/user.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "util/error.hpp"
#include "acl/misc.hpp"
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
  std::string userName = args.size() == 2 ? args[1] : client.User().Name();
  if (!acl::AllowSiteCmd(client.User(), "user"))
  {
    if (userName != client.User().Name() ||
        !acl::AllowSiteCmd(client.User(), "userown"))
    {
      if (!client.User().HasGadminGID(acl::NameToPrimaryGID(userName)) ||
          !acl::AllowSiteCmd(client.User(), "usergadmin"))
      {
        throw cmd::PermissionError();
      }
    }
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

  auto user = acl::User::Load(userName);
  if (!user)
  {
    control.Reply(ftp::ActionNotOkay, "User " + userName + " doesn't exist.");
    return;
  }

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  body.RegisterValue("user", user->Name());
  body.RegisterValue("logged_in", user->LoggedIn());
  body.RegisterValue("created", boost::gregorian::to_simple_string(user->Created()));

  if (user->LastLogin())
  {
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_duration diff = now - *user->LastLogin();
    body.RegisterValue("last_login", boost::posix_time::to_simple_string(*user->LastLogin()));
    body.RegisterValue("since_login", boost::posix_time::to_simple_string(diff));
  }
  else
    body.RegisterValue("last_login", "Never");

  if (user->Expires())
    body.RegisterValue("expires", boost::gregorian::to_simple_string(*user->Expires()));
  else
    body.RegisterValue("expires", "Never");
    
  body.RegisterValue("creator", acl::UIDToName(user->Creator()));
  body.RegisterValue("flags", user->Flags());
  body.RegisterValue("ratio", acl::RatioString(*user));  
  body.RegisterValue("credits", acl::CreditString(*user));
  body.RegisterValue("groups", acl::GroupString(*user));
  body.RegisterValue("tagline", user->Tagline());
  body.RegisterValue("comment", user->Comment());
  body.RegisterValue("weekly_allot", acl::WeeklyAllotmentString(*user));

  const auto& ipMasks = user->IPMasks();
  for (auto it = ipMasks.begin(); it != ipMasks.end(); ++it)
  {
    std::ostringstream tag;
    tag << "ip" << std::distance(ipMasks.begin(), it);
    body.RegisterValue(tag.str(), *it);
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

