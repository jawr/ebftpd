#include <sstream>
#include <unordered_map>
#include "cmd/site/group.hpp"
#include "util/error.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "acl/flags.hpp"
#include "util/time.hpp"
#include "stats/stat.hpp"
#include "db/stats/stats.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"
#include "acl/util.hpp"
#include "acl/misc.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void GROUPCommand::PopulateHeadOrFoot(const acl::Group& group, text::TemplateSection& tmpl)
{
  tmpl.RegisterValue("group", group.Name());
  tmpl.RegisterValue("descr", group.Description());
  tmpl.RegisterValue("slots", group.Slots() != -1 ? 
                              boost::lexical_cast<std::string>(group.Slots()) : 
                              "Unlimited");
  tmpl.RegisterValue("leechslots", group.LeechSlots() != -1 ? 
                                   boost::lexical_cast<std::string>(group.LeechSlots()) : 
                                   "Unlimited");
  tmpl.RegisterValue("allotslots", group.AllotmentSlots() != -1 ? 
                                   boost::lexical_cast<std::string>(group.AllotmentSlots()) : 
                                   "Unlimited");
  tmpl.RegisterValue("maxallotsize", group.MaxAllotmentSize());
  tmpl.RegisterValue("maxlogins", group.MaxLogins() != -1 ?
                                   boost::lexical_cast<std::string>(group.MaxLogins()) : 
                                   "Unlimited");
}

void GROUPCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "group") &&
      acl::AllowSiteCmd(client.User(), "groupgadmin") && 
      !client.User().HasGadminGID(acl::NameToGID(args[1])))
  {
    throw cmd::PermissionError();
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

  auto group = acl::Group::Load(args[1]);
  if (!group)
  {
    control.Reply(ftp::ActionNotOkay, "Group " + args[1] + " doesn't exist.");
    return;
  }

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  PopulateHeadOrFoot(*group, head);
  PopulateHeadOrFoot(*group, foot);

  auto users = acl::User::GetUsers("=" + args[1]);

  std::ostringstream os;
  os << head.Compile();

  std::unordered_map<acl::UserID, ::stats::Stat> dnStats;// = db::stats::GetAllDown(users);
  std::unordered_map<acl::UserID, ::stats::Stat> upStats;// = db::stats::GetAllUp(users);

  for (auto& user: users)
  {
    std::string flag = " ";
    if (user.HasFlag(acl::Flag::Siteop)) flag = "*";
    else if (user.HasFlag(acl::Flag::Gadmin)) flag = "+";
    else if (user.HasFlag(acl::Flag::Useredit)) flag = "%";

    body.RegisterValue("flag", flag);
    body.RegisterValue("user", user.Name());
    body.RegisterValue("files_up", upStats[user.ID()].Files());
    body.RegisterSize("size_up", upStats[user.ID()].KBytes());
    body.RegisterValue("files_dn", dnStats[user.ID()].Files());
    body.RegisterSize("size_dn", dnStats[user.ID()].KBytes());
    body.RegisterValue("ratio", acl::RatioString(user));
    body.RegisterValue("weekly_allot", user.WeeklyAllotment());
    os << body.Compile();
  }
  
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str()); 
}

// end
}
}
