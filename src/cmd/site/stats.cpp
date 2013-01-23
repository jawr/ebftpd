#include <map>
#include <boost/optional.hpp>
#include "cmd/site/stats.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "db/stats/stat.hpp"
#include "cfg/get.hpp"
#include "acl/allowsitecmd.hpp"
#include "cmd/error.hpp"
#include "acl/usercache.hpp"

namespace cmd { namespace site
{

void STATSCommand::Execute()
{
  if (args.size() == 2 && args[1] != client.User().Name() && 
      !acl::AllowSiteCmd(client.User(), "stats"))
  {
    throw cmd::PermissionError();
  }
  
  std::string userName = args.size() == 2 ? args[1] : client.User().Name();
  
  auto uid = acl::UserCache::NameToUID(userName);
  if (uid < 0)
  {
    control.Reply(ftp::ActionNotOkay, "User " + userName + " doesn't exist.");
    return;
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("stats"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;
  text::TemplateSection& head = templ->Head();
  head.RegisterValue("user", userName);
  os << head.Compile();
  
  std::map< ::stats::Timeframe, std::map< ::stats::Direction, ::stats::Stat>> totals;
  
  text::TemplateSection& body = templ->Body();
  for (const auto& kv : cfg::Get().Sections())
  {
    body.RegisterValue("section", kv.first);

    for (auto tf : ::stats::timeframes)
    {
      for (auto dir : ::stats::directions)
      {
        std::string prefix = util::EnumToString(tf) + "_" +
                             util::EnumToString(dir) + "_";
        auto stat = db::stats::CalculateSingleUser(uid, kv.first, tf, dir);
        body.RegisterValue(prefix + "files", stat.Files());
        body.RegisterSize(prefix + "bytes", stat.Bytes());
        body.RegisterSpeed(prefix + "speed", stat.Speed());
    
        totals[tf][dir].Incr(stat);
      }
    }

    os << body.Compile();
    body.Reset();
  }
  
  text::TemplateSection& foot = templ->Foot();

  for (auto tf : ::stats::timeframes)
  {
    for (auto dir : ::stats::directions)
    {
      std::string prefix = util::EnumToString(tf) + "_" +
                           util::EnumToString(dir) + "_";
      foot.RegisterValue(prefix + "files", totals[tf][dir].Files());
      foot.RegisterSize(prefix + "bytes", totals[tf][dir].Bytes());
      foot.RegisterSpeed(prefix + "speed", totals[tf][dir].Speed());
    }
  }

  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
