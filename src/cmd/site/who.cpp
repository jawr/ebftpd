#include <vector>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/who.hpp"
#include "acl/user.hpp"
#include "acl/userprofile.hpp"
#include "db/user/userprofile.hpp"
#include "acl/group.hpp"
#include "acl/groupcache.hpp"
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
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void WHOCommand::Execute()
{
  boost::unique_future<bool> future;
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

  acl::User user;
  int count = 0;
  for (auto& whoUser: whoUsers)
  {
    try
    {
      user = acl::UserCache::User(whoUser.uid);
    }
    catch (const util::RuntimeError&)
    {
      continue;
    }
    
    ++count;
    
    std::string tagline;
    try
    {
      tagline = db::userprofile::Get(whoUser.uid).Tagline();
    }
    catch (const util::RuntimeError&)
    {
    }
    
    body.RegisterValue("user", user.Name());
    body.RegisterValue("group", 
      acl::GroupCache::GIDToName(user.PrimaryGID()));
    body.RegisterValue("tagline", tagline);
    body.RegisterValue("action", whoUser.Action());
    os << body.Compile();
    body.Reset();
  }

  foot.RegisterValue("users", count);
  foot.RegisterValue("total_users", cfg.TotalUsers());
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
