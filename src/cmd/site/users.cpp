#include <boost/optional/optional.hpp>
#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/users.hpp"
#include "util/error.hpp"
#include "acl/types.hpp"
#include "acl/groupcache.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "logs/logs.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/error.hpp"

namespace cmd { namespace site
{

cmd::Result USERSCommand::Execute()
{
  std::vector<acl::User> users;
  
  if (args.size() == 2)
    users = db::user::GetByACL(args[1]);
  else
    users = db::user::GetAll();

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("users"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return cmd::Result::Okay;
  }

  std::ostringstream os;

  if (users.size() > 0)
  {
    text::TemplateSection& head = templ->Head();
    os << head.Compile();

    text::TemplateSection& body = templ->Body();

    for (auto& user: users)
    {
      acl::UserProfile profile = db::userprofile::Get(user.UID());
      std::string group = "NoGroup";
      try
      {
        acl::Group group_ = acl::GroupCache::Group(user.PrimaryGID());
        group = group_.Name();
      }
      catch (const util::RuntimeError& e)
      {
      }

      body.Reset();
      body.RegisterValue("user", user.Name());
      body.RegisterValue("group", group);
      
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("total_users", users.size());
    
    os << foot.Compile();
    
    control.Reply(ftp::CommandOkay, os.str());
  }
  else
    control.Reply(ftp::ActionNotOkay, "Error, no users found.");

  return cmd::Result::Okay;    
}

// end
}
}
