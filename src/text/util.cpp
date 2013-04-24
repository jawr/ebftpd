//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include "db/stats/stats.hpp"
#include "stats/stat.hpp"
#include "acl/util.hpp"
#include "text/parser.hpp"
#include "util/path/status.hpp"
#include "ftp/control.hpp"
#include "ftp/data.hpp"
#include "ftp/counter.hpp"
#include "acl/user.hpp"

namespace text
{

void RegisterGlobals(const ftp::Client& client, TemplateSection& ts)
{
  const cfg::Config& config = cfg::Get();
  
  std::string sectionName;
  auto section = config.SectionMatch(fs::WorkDirectory().ToString(), true);
  if (section) sectionName = section->Name();
  
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
  (void) util::path::FreeDiskSpace(fs::MakeReal(workDir).ToString(), freeSpace);
  ts.RegisterSize("free_space", freeSpace);

  ts.RegisterValue("section", !section ? "NoSection" : sectionName);
  
  if (!section || !section->SeparateCredits())
  {
    ts.RegisterSize("credits_section", client.User().SectionCredits(""));
  }
  else
  {
    ts.RegisterSize("credits_section", client.User().SectionCredits(sectionName));
  }
  
  ts.RegisterValue("username", client.User().Name());
  ts.RegisterValue("groupname", client.User().PrimaryGroup());
  
  ts.RegisterValue("flags", client.User().Flags());
  ts.RegisterValue("ratio", acl::RatioString(client.User()));
  
  ts.RegisterValue("tagline", client.User().Tagline());
  ts.RegisterValue("credits_all", acl::CreditString(client.User()));
  ts.RegisterValue("credits_current", acl::CreditString(client.User()));
  ts.RegisterValue("time_online", "");

  int onlineCount = ftp::Counter::Login().GlobalCount();
  ts.RegisterValue("online_users", onlineCount);
  ts.RegisterValue("all_online_users", onlineCount);
  
  for (auto tf : ::stats::timeframes)
  {
    for (auto dir : ::stats::directions)
    {
      std::string prefix = util::EnumToString(tf) + "_" +
                           util::EnumToString(dir) + "_";
      bool doGlobal = ts.HasTag(prefix + "files") ||
                      ts.HasTag(prefix + "size") ||
                      ts.HasTag(prefix + "speed");
      bool doSection = ts.HasTag(prefix + "files_section") ||
                       ts.HasTag(prefix + "size_section") ||
                       ts.HasTag(prefix + "speed_section");
      if (doGlobal || (doSection && !section))
      {
        auto stat = db::stats::CalculateSingleUser(client.User().ID(), "", tf, dir);
        
        if (doGlobal)
        {
          ts.RegisterValue(prefix + "files", stat.Files());
          ts.RegisterSize(prefix + "size", stat.KBytes());
          ts.RegisterSpeed(prefix + "speed", stat.Speed());
        }
        
        if (doSection && !section)
        {
          ts.RegisterValue(prefix + "files_section", stat.Files());
          ts.RegisterSize(prefix + "size_section", stat.KBytes());
          ts.RegisterSpeed(prefix + "speed_section", stat.Speed());
        }
      }
      
      if (doSection && section)
      {
        auto stat = db::stats::CalculateSingleUser(client.User().ID(), section->Name(), tf, dir);
        ts.RegisterValue(prefix + "files_section", stat.Files());
        ts.RegisterSize(prefix + "size_section", stat.KBytes());
        ts.RegisterSpeed(prefix + "speed_section", stat.Speed());
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
