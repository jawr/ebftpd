#include <boost/optional.hpp>
#include "cmd/site/help.hpp"
#include "cmd/site/factory.hpp"
#include "acl/allowsitecmd.hpp"
#include "main.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"

namespace cmd { namespace site
{

void HELPCommand::Syntax()
{
  boost::to_upper(args[1]);
  cmd::site::CommandDefOpt def(cmd::site::Factory::Lookup(args[1]));
  if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else
  {
    std::ostringstream os;
    os << def->Syntax() << "\n";
    os << "Description: " << def->Description();
    control.Reply(ftp::CommandOkay, os.str());
  }

  return;
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
      sorted.push_back(kv.first);
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
    
  const Factory::CommandDefsMap& commands = Factory::Commands();

  std::vector<std::string> sorted;
  for (const auto& kv : commands)
  {
    if (acl::AllowSiteCmd(client.User(), kv.second.ACLKeyword()))
    {
      sorted.push_back(kv.first);
    }
  }

  std::sort(sorted.begin(), sorted.end());

  std::ostringstream os;
  os << head.Compile();
  
  for (const auto& command : sorted)
  {
    body.RegisterValue("command", command);
    body.RegisterValue("description", commands.at(command).Description());
    os << body.Compile();
    body.Reset();
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
