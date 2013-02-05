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
#include "db/group/groupprofile.hpp"
#include "util/time.hpp"
#include "stats/stat.hpp"
#include "db/stats/stat.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "text/template.hpp"
#include "text/templatesection.hpp"
#include "text/tag.hpp"
#include "acl/groupprofile.hpp"
#include "acl/util.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void GROUPCommand::PopulateHeadOrFoot(const acl::Group& group, 
      const acl::GroupProfile& profile, text::TemplateSection& tmpl)
{
  tmpl.RegisterValue("group", group.Name());
  tmpl.RegisterValue("descr", profile.Description());
  tmpl.RegisterValue("slots", profile.Slots() != -1 ? 
                              boost::lexical_cast<std::string>(profile.Slots()) : 
                              "Unlimited");
  tmpl.RegisterValue("leechslots", profile.LeechSlots() != -1 ? 
                                   boost::lexical_cast<std::string>(profile.LeechSlots()) : 
                                   "Unlimited");
  tmpl.RegisterValue("allotslots", profile.AllotSlots() != -1 ? 
                                   boost::lexical_cast<std::string>(profile.AllotSlots()) : 
                                   "Unlimited");
  tmpl.RegisterValue("maxallotsize", profile.MaxAllotSize());
  tmpl.RegisterValue("maxlogins", profile.MaxLogins() != -1 ?
                                   boost::lexical_cast<std::string>(profile.MaxLogins()) : 
                                   "Unlimited");
}

void GROUPCommand::Execute()
{
  if (!acl::AllowSiteCmd(client.User(), "group") &&
      acl::AllowSiteCmd(client.User(), "groupgadmin") && 
      !client.User().HasGadminGID(acl::GroupCache::NameToGID(args[1])))
  {
    throw cmd::PermissionError();
  }

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
  
  acl::GroupProfile profile;
  try
  {
    profile = db::groupprofile::Get(group.GID());
  }
  catch (const util::RuntimeError&)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to load group profile for " + group.Name() + ".");
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

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  PopulateHeadOrFoot(group, profile, head);
  PopulateHeadOrFoot(group, profile, foot);
    
  std::vector<acl::User> users = db::user::GetByACL("=" + args[1]);

  std::ostringstream os;
  os << head.Compile();

  std::unordered_map<acl::UserID, acl::UserProfile> profiles = 
    db::userprofile::GetSelection(users);

  std::unordered_map<acl::UserID, ::stats::Stat> dnStats;// = db::stats::GetAllDown(users);
  std::unordered_map<acl::UserID, ::stats::Stat> upStats;// = db::stats::GetAllUp(users);

  std::cout << db::groupprofile::SlotsUsed(group.GID()) << std::endl;
  
  for (auto& user: users)
  {
    std::string flag = " ";
    if (user.CheckFlag(acl::Flag::Siteop)) flag = "*";
    else if (user.CheckFlag(acl::Flag::Gadmin)) flag = "+";
    else if (user.CheckFlag(acl::Flag::Useredit)) flag = "%";

    body.RegisterValue("flag", flag);
    body.RegisterValue("user", user.Name());
    body.RegisterValue("files_up", upStats[user.UID()].Files());
    body.RegisterSize("size_up", upStats[user.UID()].KBytes());
    body.RegisterValue("files_dn", dnStats[user.UID()].Files());
    body.RegisterSize("size_dn", dnStats[user.UID()].KBytes());
    body.RegisterValue("ratio", acl::RatioString(profiles[user.UID()]));
    body.RegisterValue("weekly_allot", profiles[user.UID()].WeeklyAllotment());
    os << body.Compile();
  }
  
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str()); 
}

// end
}
}
