#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/gpranks.hpp"
#include "db/stats/stat.hpp"
#include "stats/types.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "acl/usercache.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "acl/groupcache.hpp"
#include "logs/logs.hpp"
#include "db/group/groupprofile.hpp"
#include "acl/allowsitecmd.hpp"
#include "acl/misc.hpp"
#include "acl/group.hpp"
#include "stats/stat.hpp"

namespace cmd { namespace site

{

void GPRANKSCommand::Execute()
{
  if (args[0] == "RANKS")
  {
    if (!acl::AllowSiteCmd(client.User(), "gpranks")) throw cmd::PermissionError();
  }
  else
  {
    if (!acl::AllowSiteCmd(client.User(), "gpranksalias")) throw cmd::PermissionError();
  }

  ::stats::Timeframe tf;
  if (!util::EnumFromString(args[1], tf)) throw cmd::SyntaxError();

  ::stats::Direction dir;
  if (!util::EnumFromString(args[2], dir)) throw cmd::SyntaxError();
  
  ::stats::SortField sf;
  if (!util::EnumFromString(args[3], sf)) throw cmd::SyntaxError();

  int number = 10;
  if (args.size() >= 5)
  {
    try
    {
      number = boost::lexical_cast<int>(args[4]);
      if (number < 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
  }
  
  int maxNumber = acl::stats::MaxGroups(client.User());
  if (maxNumber != -1)
  {
    number = std::min(maxNumber, number);
  }
  
  const cfg::Config& config = cfg::Get();

  std::string section;
  if (args.size() >= 6)
  {
    section = boost::to_upper_copy(args[5]);
    if (config.Sections().find(section) == config.Sections().end())
    {
      control.Reply(ftp::ActionNotOkay, "Section " + section + " doesn't exist.");
      return;
    }
  }
  
  auto groups = ::db::stats::CalculateGroupRanks(section, tf, dir, sf);
  
  std::string tmplName = "gpranks." + util::EnumToString(tf) + 
                         "." + util::EnumToString(dir) + 
                         "." + util::EnumToString(sf);
                         
  boost::optional<text::Template> templ;
  try
  {
    try
    {
      templ.reset(text::Factory::GetTemplate(tmplName));
    }
    catch (const text::TemplateError&)
    {
      templ.reset(text::Factory::GetTemplate("gpranks"));
    }
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  std::ostringstream os;

  text::TemplateSection& head = templ->Head();
  head.RegisterValue("section", section.empty() ? "ALL" : section);
  os << head.Compile();
  
  text::TemplateSection& body = templ->Body();

  long long totalKBytes = 0;
  long long totalFiles = 0;
  long long totalXfertime = 0;

  int index = 0;
  for (const auto& g : groups)
  {
    if (index < number)
    {
      std::string description;
      try
      {
        description = db::groupprofile::Get(g.ID()).Description();
      }
      catch (const util::RuntimeError&)
      {
      }
      
      body.RegisterValue("index", ++index);
      body.RegisterValue("group", acl::GroupCache::GIDToName(g.ID()));
      body.RegisterValue("descr", description);
      body.RegisterValue("files", g.Files());
      body.RegisterSize("size", g.KBytes());
      body.RegisterSpeed("speed", g.Speed());
      
      os << body.Compile();
    }
    
    totalKBytes += g.KBytes();
    totalFiles += g.Files();
    totalXfertime += g.Xfertime();
  }
  
  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("groups", groups.size());
  foot.RegisterSize("size", totalKBytes);
  foot.RegisterValue("files" ,totalFiles);
  foot.RegisterSpeed("speed", totalXfertime == 0 ? totalKBytes : totalKBytes / (totalXfertime / 1000.0));
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
