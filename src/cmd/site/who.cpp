#include <vector>
#include <future>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/who.hpp"
#include "acl/user.hpp"
#include "ftp/task/types.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "stats/util.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

void WHOCommand::Execute()
{
  std::future<bool> future;
  std::vector<ftp::task::WhoUser> whoUsers;

  std::make_shared<ftp::task::GetOnlineUsers>(whoUsers, future)->Push();

  const cfg::Config& cfg = cfg::Get();

  future.wait();

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("who"));
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
  
  head.RegisterValue("sitename_short", cfg.SitenameShort());
  os << head.Compile();

  int count = 0;
  for (auto& whoUser : whoUsers)
  {
    auto user = acl::User::Load(whoUser.uid);
    if (!user) continue;
    
    ++count;
    
    body.RegisterValue("user", user->Name());
    body.RegisterValue("group", user->PrimaryGroup());
    body.RegisterValue("tagline", user->Tagline());
    body.RegisterValue("action", whoUser.Action());
    os << body.Compile();
  }

  foot.RegisterValue("online_users", count);
  foot.RegisterValue("all_online_users", count);
  foot.RegisterValue("max_online_users", cfg::Get().MaxUsers().Users());
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
