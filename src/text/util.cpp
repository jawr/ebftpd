#include <cassert>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "text/util.hpp"
#include "fs/path.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "ftp/client.hpp"
#include "cfg/get.hpp"
#include "fs/directory.hpp"
#include "ftp/task/task.hpp"
#include "acl/groupcache.hpp"
#include "db/stats/stat.hpp"
#include "stats/stat.hpp"

namespace text
{

void RegisterGlobals(const ftp::Client& client, TemplateSection& ts)
{
  boost::unique_future<void> oucFuture;
  auto ocuTask = std::make_shared<ftp::task::OnlineUserCount>(oucFuture);
  ocuTask->Push();

  const cfg::Config& config = cfg::Get();

  ts.RegisterSpeed("last_speed", 0);
  ts.RegisterValue("max_online_users", config.MaxUsers().Users());

  auto now = boost::posix_time::second_clock::local_time();
  std::string timeStr(boost::posix_time::to_simple_string(now));
  auto pos = timeStr.find(' ');
  assert(pos != std::string::npos);

  ts.RegisterValue("date_time", timeStr);
  ts.RegisterValue("date", timeStr.substr(0, pos));
  ts.RegisterValue("time", timeStr.substr(pos + 1));
  
  ts.RegisterValue("remote_host", client.Hostname());
  ts.RegisterValue("remote_ip", client.IP());
  ts.RegisterValue("ident", client.Ident());
  
  ts.RegisterValue("sitename_long", config.SitenameLong());
  ts.RegisterValue("sitename_short", config.SitenameShort());

  const fs::VirtualPath& workDir = fs::WorkDirectory();
  ts.RegisterValue("work_dir", workDir.ToString());
  ts.RegisterSize("free_space", 0);
  auto section = config.SectionMatch(workDir);
  ts.RegisterValue("section", section ? section->Name() : "");
  
  ts.RegisterValue("username", client.User().Name());
  ts.RegisterValue("groupname", acl::GroupCache::GIDToName(client.User().PrimaryGID()));
  ts.RegisterValue("flags", client.User().Flags());
  
  if (client.UserProfile().Ratio() == 0)
    ts.RegisterValue("ratio", "Unlimited");
  else
  {
    std::ostringstream ratio;
    ratio << "1:" << client.UserProfile().Ratio();
    ts.RegisterValue("ratio", ratio.str());
  }
  
  ts.RegisterValue("tagline", client.User().Tagline());
  ts.RegisterSize("credits", client.User().Credits());
  ts.RegisterValue("time_online", "");
  //boost::wait_for_all(oucFuture);
  
  ts.RegisterValue("online_users", ocuTask->Count());
  ts.RegisterValue("all_online_users", ocuTask->AllCount());
  
  for (auto tf : ::stats::timeframes)
  {
    for (auto dir : ::stats::directions)
    {
      std::string prefix = util::EnumToString(tf) + "_" +
                           util::EnumToString(dir) + "_";
      auto stat = db::stats::CalculateSingleUser(client.User().UID(), "", tf, dir);
      ts.RegisterValue(prefix + "files", stat.Files());
      ts.RegisterSize(prefix + "bytes", stat.Bytes());
      ts.RegisterSpeed(prefix + "speed", stat.Speed());
    }
  }
}

util::Error GenericTemplate(const ftp::Client& client, const std::string& name, std::string& messages)
{
  try
  {
    Template tmpl = Factory::GetTemplate(name);
    
    // !!!!!!! UGLY running register globals 3 times.
    // need a mechanism for duplicating the registrations across
    // all sections
    
    RegisterGlobals(client, tmpl.Head());
    RegisterGlobals(client, tmpl.Body());
    RegisterGlobals(client, tmpl.Foot());
    
    std::ostringstream os;
    os << tmpl.Head().Compile();
    os << tmpl.Body().Compile();
    os << tmpl.Foot().Compile();
    
    messages = os.str();
  }
  catch (const TemplateError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

} /* text namespace */
