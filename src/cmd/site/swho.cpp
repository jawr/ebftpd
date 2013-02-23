#include <vector>
#include <future>
#include <boost/optional.hpp>
#include "cmd/site/swho.hpp"
#include "acl/user.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "stats/util.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

void SWHOCommand::Execute()
{
  std::future<bool> future;
  std::vector<ftp::task::WhoUser> whoUsers;

  std::make_shared<ftp::task::GetOnlineUsers>(whoUsers, future)->Push();
  future.wait();

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("swho"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  const cfg::Config& cfg = cfg::Get();
  head.RegisterValue("sitename_short", cfg.SitenameShort());
  os << head.Compile();

  for (auto& whoUser : whoUsers)
  {
    body.RegisterValue("user", acl::UIDToName(whoUser.uid));
    body.RegisterValue("ident_address", whoUser.ident + "@" + whoUser.address);
    body.RegisterValue("action", whoUser.Action());
    os << body.Compile();
  }

  foot.RegisterValue("users", whoUsers.size());
  foot.RegisterValue("total_users", cfg.TotalUsers());
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
  return; 
}

// end
}
}
