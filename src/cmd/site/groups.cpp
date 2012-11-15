#include <sstream>
#include <vector>
#include <boost/optional.hpp>
#include "cmd/site/groups.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "db/user/user.hpp"
#include "db/group/group.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

void GROUPSCommand::Execute()
{
  std::vector<acl::Group> groups = db::group::GetAll();

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("groups"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;

  if (groups.size() > 0)
  {
    text::TemplateSection& head = templ->Head();
    os << head.Compile();

    text::TemplateSection& body = templ->Body();

    for (auto& group: groups)
    {
      std::vector<acl::User> users = db::user::GetByACL("=" + group.Name());

      body.Reset();
      body.RegisterValue("users", users.size());
      body.RegisterValue("group", group.Name());
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("total_groups", groups.size());
    os << foot.Compile();
  }
  else
    os << "No groups added.";

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
