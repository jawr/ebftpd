#include <utility>
#include <iomanip>
#include <boost/optional.hpp>
#include "cmd/site/help.hpp"
#include "cmd/site/factory.hpp"
#include "acl/misc.hpp"
#include "main.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void HELPCommand::Syntax()
{
  util::ToUpper(args[1]);
  auto def(cmd::site::Factory::Lookup(client, args[1]));
  if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else
  {
    std::ostringstream os;
    os << def->Syntax() << "\n";
    os << "Description: " << def->Description();
    control.Reply(ftp::CommandOkay, os.str());
  }
}

void HELPCommand::ListNoTemplate()
{
  const Factory::CommandDefsMap& commands = Factory::Commands();

  std::ostringstream os;
  os << " " << programFullname << " SITE command listing - \n\n";
  
  std::vector<std::string> sorted;
  size_t maxLen = 0;
  for (auto& kv : commands)
  {
    if (acl::AllowSiteCmd(client.User(), kv.second.ACLKeyword()))
    {
      sorted.emplace_back(kv.first);
      maxLen = std::max(kv.first.length(), maxLen);
    }
  }
  
  std::sort(sorted.begin(), sorted.end());
    
  for (auto& command : sorted)
  {
    os << " " << std::setw(maxLen) << command << " : " 
       << commands.at(command).Description() << "\n";
  }
    
  os << "\n End of list";
  control.Reply(ftp::CommandOkay, os.str());
}

void HELPCommand::List()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("help"));
  }
  catch (const text::TemplateError& e)
  {
    ListNoTemplate();
    return;
  }
  
  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();

  head.RegisterValue("server", programFullname);
  foot.RegisterValue("server", programFullname);
    
  std::vector<std::pair<std::string, std::string>> sorted;
  for (const auto& kv : Factory::Commands())
  {
    if (acl::AllowSiteCmd(client.User(), kv.second.ACLKeyword()))
    {
      sorted.emplace_back(kv.first, kv.second.Description());
    }
  }
  
  for (const auto& sc : cfg::Get().SiteCmd())
  {
    auto def = Factory::LookupCustom(sc.Command());
    if (def && acl::AllowSiteCmd(client.User(), def->ACLKeyword()))
      sorted.emplace_back(sc.Command(), sc.Description());
  }

  std::sort(sorted.begin(), sorted.end(),
        [](const std::pair<std::string, std::string>& c1,
           const std::pair<std::string, std::string>& c2)
        {
          return c1.first < c2.first;
        });

  std::ostringstream os;
  os << head.Compile();
  
  for (const auto& command : sorted)
  {
    body.RegisterValue("command", command.first);
    body.RegisterValue("descr", command.second);
    os << body.Compile();
  }
  
  os << foot.Compile();
  control.Reply(ftp::CommandOkay, os.str());
}

void HELPCommand::Execute()
{
  if (args.size() == 2) return Syntax();
  else return List();
}

} /* site namespace */
} /* cmd namespace */
