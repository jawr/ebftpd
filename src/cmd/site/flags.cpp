#include <sstream>
#include <boost/optional.hpp>
#include "cmd/site/flags.hpp"
#include "acl/user.hpp"
#include "acl/usercache.hpp"
#include "acl/flags.hpp"
#include "util/error.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

void FLAGSCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() &&
      !acl::AllowSiteCmd(client.User(), "flags"))
  {
    throw cmd::PermissionError();
  }

  acl::User user(client.User());
  if (args.size() == 2)
  {
    try
    {
      user = acl::UserCache::User(args[1]);
    }
    catch (const util::RuntimeError& e)
    {
      control.Reply(ftp::ActionNotOkay, e.Message());
      return;
    }
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
  
  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();
  
  body.RegisterValue("user", user.Name());
  body.RegisterValue("flag1", CheckFlag(user, acl::Flag::Siteop));
  body.RegisterValue("flag2", CheckFlag(user, acl::Flag::Gadmin));
  body.RegisterValue("flag3", CheckFlag(user, acl::Flag::Template));
  body.RegisterValue("flag4", CheckFlag(user, acl::Flag::Exempt));
  body.RegisterValue("flag5", CheckFlag(user, acl::Flag::Color));
  body.RegisterValue("flag6", CheckFlag(user, acl::Flag::Deleted));
  body.RegisterValue("flag7", CheckFlag(user, acl::Flag::Useredit));
  body.RegisterValue("flag8", CheckFlag(user, acl::Flag::Anonymous));

  body.RegisterValue("flaga", CheckFlag(user, acl::Flag::Nuke));
  body.RegisterValue("flagb", CheckFlag(user, acl::Flag::Unnuke));
  body.RegisterValue("flagc", CheckFlag(user, acl::Flag::Undupe));
  body.RegisterValue("flagd", CheckFlag(user, acl::Flag::Kick));
  body.RegisterValue("flage", CheckFlag(user, acl::Flag::Kill));
  body.RegisterValue("flagf", CheckFlag(user, acl::Flag::Take));
  body.RegisterValue("flagg", CheckFlag(user, acl::Flag::Give));
  body.RegisterValue("flagh", CheckFlag(user, acl::Flag::Users));
  body.RegisterValue("flagi", CheckFlag(user, acl::Flag::Idler));
  body.RegisterValue("flagj", CheckFlag(user, acl::Flag::Custom1));
  body.RegisterValue("flagk", CheckFlag(user, acl::Flag::Custom2));
  body.RegisterValue("flagl", CheckFlag(user, acl::Flag::Custom3));
  body.RegisterValue("flagm", CheckFlag(user, acl::Flag::Custom4));
  body.RegisterValue("flagn", CheckFlag(user, acl::Flag::Custom5));

  std::ostringstream os;
  os << head.Compile();
  os << body.Compile();
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
