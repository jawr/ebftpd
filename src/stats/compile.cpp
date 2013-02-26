#include <boost/optional.hpp>
#include "stats/compile.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "db/stats/stats.hpp"
#include "stats/stat.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"

namespace stats
{

std::string CompileUserRanks(const std::string& section, Timeframe tf, Direction dir, 
                             SortField sf, int max, text::Template& templ, 
                             const std::function<bool(acl::User&)>& filter)
{
  auto users = ::db::stats::CalculateUserRanks(section, tf, dir, sf);
  
  std::ostringstream os;
  text::TemplateSection& head = templ.Head();
  head.RegisterValue("section", section.empty() ? "ALL" : section);
  os << head.Compile();
  
  text::TemplateSection& body = templ.Body();

  long long totalBytes = 0;
  long long totalFiles = 0;
  long long totalXfertime = 0;

  int index = 0;
  unsigned total = 0;
  for (const auto& u : users)
  {
    if (u.Files() <= 0) break;
    if (index < max)
    {
      auto user = acl::User::Load(u.ID());      
      if (!user || (filter && !filter(*user))) continue;
      
      body.RegisterValue("index", ++index);
      body.RegisterValue("user", user->Name());
      body.RegisterValue("group", user->PrimaryGroup());
      body.RegisterValue("tagline", user->Tagline());
      body.RegisterValue("files", u.Files());
      body.RegisterSize("size", u.KBytes());
      body.RegisterSpeed("speed", u.Speed());
      os << body.Compile();
    }
    
    totalBytes += u.KBytes();
    totalFiles += u.Files();
    totalXfertime += u.Xfertime();
    ++total;
  }
  
  text::TemplateSection& foot = templ.Foot();
  foot.RegisterValue("users", total);
  foot.RegisterSize("size", totalBytes);
  foot.RegisterValue("files" ,totalFiles);
  foot.RegisterSpeed("speed", totalXfertime == 0 ? totalBytes : totalBytes / (totalXfertime / 1000.0));
  os << foot.Compile();
  return os.str();
}

std::string CompileGroupRanks(const std::string& section, Timeframe tf, Direction dir, 
                              SortField sf, int max, text::Template& templ)
{
  auto groups = ::db::stats::CalculateGroupRanks(section, tf, dir, sf);
  
  std::ostringstream os;
  text::TemplateSection& head = templ.Head();
  head.RegisterValue("section", section.empty() ? "ALL" : section);
  os << head.Compile();
  
  text::TemplateSection& body = templ.Body();

  long long totalKBytes = 0;
  long long totalFiles = 0;
  long long totalXfertime = 0;

  int index = 0;
  unsigned total = 0;
  for (const auto& g : groups)
  {
    if (g.Files() <= 0) break;
    if (index < max)
    {
      auto group = acl::Group::Load(g.ID());
      body.RegisterValue("index", ++index);
      body.RegisterValue("group", group ? group->Name() : "unknown");
      body.RegisterValue("descr", group ? group->Description() : "");
      body.RegisterValue("files", g.Files());
      body.RegisterSize("size", g.KBytes());
      body.RegisterSpeed("speed", g.Speed());
      
      os << body.Compile();
    }
    
    totalKBytes += g.KBytes();
    totalFiles += g.Files();
    totalXfertime += g.Xfertime();
    ++total;
  }
  
  text::TemplateSection& foot = templ.Foot();
  foot.RegisterValue("groups", total);
  foot.RegisterSize("size", totalKBytes);
  foot.RegisterValue("files" ,totalFiles);
  foot.RegisterSpeed("speed", totalXfertime == 0 ? totalKBytes : totalKBytes / (totalXfertime / 1000.0));
  os << foot.Compile();
  
  return os.str();
}

} /* stats namespace */
