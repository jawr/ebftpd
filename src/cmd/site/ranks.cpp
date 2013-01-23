#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/ranks.hpp"
#include "db/stats/stat.hpp"
#include "stats/types.hpp"
#include "cmd/error.hpp"
#include "cfg/get.hpp"
#include "acl/usercache.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "acl/groupcache.hpp"
#include "logs/logs.hpp"
#include "acl/acl.hpp"

namespace cmd { namespace site
{

void RANKSCommand::Execute()
{
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
  
  acl::ACL acl(acl::ACL::FromString("*"));
  if (args.size() >= 7)
  {
    std::vector<std::string> aclArgs(args.begin() + 6, args.end());
    acl = acl::ACL::FromString(boost::join(aclArgs, " "));
  }
  
  auto users = ::db::stats::CalculateUserRanks(section, tf, dir, sf);
  
  std::string tmplName = "ranks." + util::EnumToString(tf) + 
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
      templ.reset(text::Factory::GetTemplate("ranks"));
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

  long long totalBytes = 0;
  long long totalFiles = 0;
  long long totalXfertime = 0;

  acl::User user;
  int index = 0;
  for (const auto& u : users)
  {
    if (index < number)
    {
      try
      {
        user = acl::UserCache::User(u.ID());
      }
      catch (const util::RuntimeError&)
      {
        // exclude user from stats
        logs::error << "Unable to load user with uid from user cache: " 
                    << u.ID() << logs::endl;
        continue;
      }
      
      if (!acl.Evaluate(user)) continue;
      
      body.RegisterValue("index", ++index);
      body.RegisterValue("user", user.Name());
      body.RegisterValue("group", acl::GroupCache::GIDToName(user.PrimaryGID()));
      body.RegisterValue("tagline", user.Tagline());
      body.RegisterValue("files", u.Files());
      body.RegisterSize("bytes", u.Bytes());
      body.RegisterSpeed("speed", u.Speed());
      os << body.Compile();
      body.Reset();
    }
    
    totalBytes += u.Bytes();
    totalFiles += u.Files();
    totalXfertime += u.Xfertime();
  }
  
  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("users", users.size());
  foot.RegisterSize("bytes", totalBytes);
  foot.RegisterValue("files" ,totalFiles);
  foot.RegisterSpeed("speed", totalXfertime == 0 ? totalBytes : totalBytes / (totalXfertime / 1000.0));
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
