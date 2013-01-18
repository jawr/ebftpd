#include <sstream>
#include <unordered_map>
#include "cmd/site/group.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "acl/flags.hpp"
#include "db/user/user.hpp"
#include "db/user/userprofile.hpp"
#include "util/time.hpp"
#include "stats/stat.hpp"
#include "db/stats/stat.hpp"
#include "stats/conversions.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"

namespace cmd { namespace site
{

void GROUPCommand::Execute()
{
  acl::Group group;
  try
  {
    group = acl::GroupCache::Group(args[1]);
  }
  catch (const util::RuntimeError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::vector<acl::User> users = db::user::GetByACL("=" + args[1]);

  if (users.empty())
  {
    control.Reply(ftp::ActionNotOkay, "SITE GROUP: No users.");
    return;
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("group"));
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
  
  os << head.Compile();

  std::unordered_map<acl::UserID, acl::UserProfile> profiles = 
    db::userprofile::GetSelection(users);

  std::unordered_map<acl::UserID, ::stats::Stat> dnStats = db::stats::GetAllDown(users);
  std::unordered_map<acl::UserID, ::stats::Stat> upStats = db::stats::GetAllUp(users);

  for (auto& user: users)
  {
    body.Reset();
    std::string flag = (user.CheckFlag(acl::Flag::Gadmin)) ? "+" : " ";
    flag = (user.CheckFlag(acl::Flag::Siteop)) ? "*" : flag;
    body.RegisterValue("flag", flag);
    body.RegisterValue("user", user.Name());
    body.RegisterValue("files_up", upStats[user.UID()].Files());
    body.RegisterSize("amount_up", upStats[user.UID()].Bytes());
    body.RegisterValue("files_dn", dnStats[user.UID()].Files());
    body.RegisterSize("amount_dn", dnStats[user.UID()].Bytes());
    body.RegisterValue("ratio", profiles[user.UID()].Ratio());
    body.RegisterValue("weekly_allot", profiles[user.UID()].WeeklyAllotment());
    os << body.Compile();
  }
  
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str()); 
}

// end
}
}
