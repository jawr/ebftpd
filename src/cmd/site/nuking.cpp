#include <sstream>
#include <iomanip>
#include <cerrno>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "cmd/site/nuking.hpp"
#include "cmd/error.hpp"
#include "db/nuking/nuking.hpp"
#include "text/factory.hpp"
#include "util/string.hpp"
#include "text/error.hpp"
#include "cmd/util.hpp"
#include "fs/path.hpp"
#include "acl/user.hpp"
#include "util/path/extattr.hpp"
#include "logs/logs.hpp"
#include "db/stats/stats.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void NUKECommand::Execute()
{
  control.Reply(ftp::NotImplemented, "Not implemented");
}

std::string GetNukeID(const fs::RealPath& path)
{
  static const char* nukeIdAttributeName = "user.ebftpd.nukeid";
  
  char buf[25];
  int len = getxattr(path.CString(), nukeIdAttributeName, buf, sizeof(buf));
  if (len < 0)
  {
    if (errno != ENOATTR && errno != ENODATA && errno != ENOENT)
    {
      logs::Error("Error while reading filesystem attribute %1%: %2%: %3%", 
                  nukeIdAttributeName, path, util::Error::Failure(errno).Message());
    }
    return "";
  }
  
  buf[len] = '\0';
  return buf;
}

void UNNUKECommand::Execute()
{
  fs::VirtualPath path(fs::PathFromUser(args[1]));
  
  fs::RealPath real(fs::MakeReal(path));
  std::string id = GetNukeID(real);
  if (id.empty())
  {
    control.Reply(ftp::ActionNotOkay, "Unable to determine nuke id for: " + args[1]);
    return;
  }
  
  auto nuke = db::nuking::LookupNuke(id);
  if (!nuke)
  {
    control.Reply(ftp::ActionNotOkay, "Unable to retrieve nuke data for: " + args[1]);
    return;
  }
  
  const cfg::Config& config = cfg::Get();
  auto it = config.Sections().find(nuke->Section());
  boost::optional<const cfg::Section&> section;
  if (it != config.Sections().end())
  {
    section.reset(it->second);
  }
  
  for (const auto& nukee : nuke->Nukees())
  {
    auto user = acl::User::Load(nukee.UID());
    if (user)
    {
      db::stats::Unnuke(nukee.UID(), nukee.KBytes(), nukee.Files(), 
                        nuke->ModTime(), section ? section->Name() : "");
      user->IncrSectionCredits(section && section->SeparateCredits() ? section->Name() : "", 
                               nukee.Credits());
    }
  }
  
  db::nuking::DelNuke(*nuke);
  db::nuking::AddUnnuke(*nuke);
  control.Reply(ftp::CommandOkay, "Unnuke okay!");
}

std::string FormatNukees(const std::vector<db::nuking::Nukee>& nukees)
{
  std::ostringstream os;
  for (const auto& nukee : nukees)
  {
    if (!os.str().empty()) os << ", ";
    os << acl::UIDToName(nukee.UID()) 
       << std::setprecision(2) << std::fixed << (nukee.KBytes() / 1024.0) << "MB";
  }
  return os.str();
}

void NUKESCommand::Execute()
{
  int number = 10;
  if (args.size() == 2)
  {
    try
    {
      number = boost::lexical_cast<int>(args[1]);
      if (number <= 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
  }

  auto nukes = args[0] == "NUKES" ? 
               db::nuking::NewestNukes(number) :
               db::nuking::NewestUnnukes(number);

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate(util::ToLowerCopy(args[0])));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }
  
  std::ostringstream os;
  os << templ->Head().Compile();

  text::TemplateSection& body = templ->Body();

  auto now = boost::posix_time::second_clock::local_time();
  unsigned index = 0;
  for (const auto& nuke : nukes)
  {
    body.RegisterValue("index", ++index);
    body.RegisterValue("datetime", boost::lexical_cast<std::string>(nuke.DateTime()));
    body.RegisterValue("age", Age(now - nuke.DateTime()));
    body.RegisterValue("path", fs::Path(nuke.Path()).Basename().ToString());
    body.RegisterValue("section", nuke.Section());
    body.RegisterValue("nukees", FormatNukees(nuke.Nukees()));
    os << body.Compile();
  }

  text::TemplateSection& foot = templ->Foot();
  foot.RegisterValue("count", nukes.size());
  os << foot.Compile();
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
