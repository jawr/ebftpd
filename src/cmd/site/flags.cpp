#include <sstream>
#include <boost/optional.hpp>
#include "cmd/site/flags.hpp"
#include "acl/user.hpp"
#include "acl/flags.hpp"
#include "util/error.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

std::string HasFlag(const acl::User& user, const acl::Flag& flag)
{
  return user.HasFlag(flag) ? "*" : " ";
}

void FLAGSCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() &&
      !acl::AllowSiteCmd(client.User(), "flags"))
  {
    throw cmd::PermissionError();
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("flags"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  boost::optional<acl::User> user(client.User());
  if (args.size() == 2)
  {
    user = acl::User::Load(args[1]);
    if (!user)
    {
      control.Reply(ftp::ActionNotOkay, "User " + args[1] + " doesn't exist.");
      return;
    }
  }
  
  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();
  
  body.RegisterValue("user", user->Name());
  body.RegisterValue("flag1", HasFlag(*user, acl::Flag::Siteop));
  body.RegisterValue("flag2", HasFlag(*user, acl::Flag::Gadmin));
  body.RegisterValue("flag3", HasFlag(*user, acl::Flag::Template));
  body.RegisterValue("flag4", HasFlag(*user, acl::Flag::Exempt));
  body.RegisterValue("flag5", HasFlag(*user, acl::Flag::Color));
  body.RegisterValue("flag6", HasFlag(*user, acl::Flag::Deleted));
  body.RegisterValue("flag7", HasFlag(*user, acl::Flag::Useredit));
  body.RegisterValue("flag8", HasFlag(*user, acl::Flag::Anonymous));

  body.RegisterValue("flaga", HasFlag(*user, acl::Flag::Nuke));
  body.RegisterValue("flagb", HasFlag(*user, acl::Flag::Unnuke));
  body.RegisterValue("flagc", HasFlag(*user, acl::Flag::Undupe));
  body.RegisterValue("flagd", HasFlag(*user, acl::Flag::Kick));
  body.RegisterValue("flage", HasFlag(*user, acl::Flag::Kill));
  body.RegisterValue("flagf", HasFlag(*user, acl::Flag::Take));
  body.RegisterValue("flagg", HasFlag(*user, acl::Flag::Give));
  body.RegisterValue("flagh", HasFlag(*user, acl::Flag::Users));
  body.RegisterValue("flagi", HasFlag(*user, acl::Flag::Idler));
  body.RegisterValue("flagj", HasFlag(*user, acl::Flag::Custom1));
  body.RegisterValue("flagk", HasFlag(*user, acl::Flag::Custom2));
  body.RegisterValue("flagl", HasFlag(*user, acl::Flag::Custom3));
  body.RegisterValue("flagm", HasFlag(*user, acl::Flag::Custom4));
  body.RegisterValue("flagn", HasFlag(*user, acl::Flag::Custom5));

  std::ostringstream os;
  os << head.Compile();
  os << body.Compile();
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
