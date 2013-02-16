#include <sstream>
#include <vector>
#include <boost/optional.hpp>
#include "cmd/site/groups.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "db/user/user.hpp"
#include "db/group/group.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"
#include "db/group/groupprofile.hpp"
#include "logs/logs.hpp"

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
  text::TemplateSection& head = templ->Head();
  os << head.Compile();

  text::TemplateSection& body = templ->Body();

  for (auto& group: groups)
  {
    std::string description;
    try
    {
      description = db::groupprofile::Get(group.ID()).Description();
    }
    catch (const util::RuntimeError& e)
    {
      logs::error << "Unable to load group profile for: " 
                  << group.Name() << logs::endl;
    }
    
    std::vector<acl::User> users = db::user::GetByACL("=" + group.Name());

    body.RegisterValue("users", users.size());
    body.RegisterValue("group", group.Name());
    body.RegisterValue("descr", description);
    os << body.Compile();
  }

  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("total_groups", groups.size());
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
