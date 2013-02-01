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
#include "acl/util.hpp"
#include "text/parser.hpp"

namespace text
{

void RegisterGlobals(const ftp::Client& client, TemplateSection& ts)
{
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
  
  unsigned long long freeSpace = -1;
  (void) fs::FreeDiskSpace(fs::MakeReal(workDir), freeSpace);
  ts.RegisterSize("free_space", freeSpace);

  if (ts.HasTag("section"))
  {
    auto section = config.SectionMatch(workDir);
    ts.RegisterValue("section", section ? section->Name() : "");
  }
  
  ts.RegisterValue("username", client.User().Name());
  if (ts.HasTag("groupname")) 
  {
    ts.RegisterValue("groupname", acl::GroupCache::GIDToName(client.User().PrimaryGID()));
  }
  ts.RegisterValue("flags", client.User().Flags());
  ts.RegisterValue("ratio", acl::RatioString(client.UserProfile()));
  
  ts.RegisterValue("tagline", client.UserProfile().Tagline());
  ts.RegisterValue("credits", acl::CreditString(client.UserProfile()));
  ts.RegisterValue("time_online", "");

  if (ts.HasTag("online_users") || ts.HasTag("all_online_users"))
  {
    boost::unique_future<void> oucFuture;
    auto ocuTask = std::make_shared<ftp::task::OnlineUserCount>(oucFuture);
    ocuTask->Push();
    oucFuture.wait();
    
    ts.RegisterValue("online_users", ocuTask->Count());
    ts.RegisterValue("all_online_users", ocuTask->AllCount());
  }
  
  for (auto tf : ::stats::timeframes)
  {
    for (auto dir : ::stats::directions)
    {
      std::string prefix = util::EnumToString(tf) + "_" +
                           util::EnumToString(dir) + "_";
      if (ts.HasTag(prefix + "files") ||
          ts.HasTag(prefix + "size") ||
          ts.HasTag(prefix + "speed"))
      {
        auto stat = db::stats::CalculateSingleUser(client.User().UID(), "", tf, dir);
        ts.RegisterValue(prefix + "files", stat.Files());
        ts.RegisterSize(prefix + "size", stat.KBytes());
        ts.RegisterSpeed(prefix + "speed", stat.Speed());
      }
    }
  }
  
  {
    std::string tlsMode;
    if (client.Control().IsTLS()) tlsMode = "control";
    if (client.Data().Protection())
    {
      if (!tlsMode.empty()) tlsMode += '&';
      tlsMode += "data";
    }
    if (tlsMode.empty()) tlsMode = "insecure";
    
    ts.RegisterValue("tls_mode", tlsMode);
    ts.RegisterValue("tls_cipher", client.Control().TLSCipher());
  }
}

util::Error GenericTemplate(const ftp::Client& client, Template& tmpl, std::string& messages)
{
  try
  {
    // need to work out an efficient way to
    // duplicate the globals across head / body / foot
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

util::Error GenericTemplate(const ftp::Client& client, const std::string& name, std::string& messages)
{
  try
  {
    Template tmpl(Factory::GetTemplate(name));
    return GenericTemplate(client, tmpl, messages);
  }
  catch (const TemplateError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

util::Error GenericTemplate(const ftp::Client& client, const fs::Path& path,
      std::string& messages)
{
  try
  {
    TemplateParser parser(path.ToString());
    Template tmpl(parser.Create());
    return GenericTemplate(client, tmpl, messages);
  }
  catch (const TemplateError& e)
  {
    return util::Error::Failure(e.Message());
  }
}

} /* text namespace */
