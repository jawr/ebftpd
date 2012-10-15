#include "cmd/site/help.hpp"
#include "cmd/site/factory.hpp"
#include "main.hpp"

namespace cmd { namespace site
{

cmd::Result HELPCommand::Syntax()
{
  boost::to_upper(args[1]);
  cmd::site::CommandDefOptRef def(cmd::site::Factory::Lookup(args[1]));
  if (!def) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else
  {
    std::ostringstream os;
    os << def->Syntax() << "\n";
    os << "Description: " << def->Description();
    control.MultiReply(ftp::CommandOkay, os.str());
  }

  return cmd::Result::Okay;
}

cmd::Result HELPCommand::List()
{
  const Factory::CommandDefsMap& commands = Factory::Commands();

  std::ostringstream os;
  os << " " << programFullname << " SITE command listing - \n\n";
  
  size_t maxLen = 0;
  for (auto& kv : commands)
    maxLen = std::max(kv.first.length(), maxLen);
    
  for (auto& kv : commands)
    os << " " << std::setw(maxLen) << kv.first << " : " << kv.second.Description() << "\n";
    
  os << "\n End of list";
  control.MultiReply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

cmd::Result HELPCommand::Execute()
{
  if (args.size() == 2) return Syntax();
  else return List();
}

} /* site namespace */
} /* cmd namespace */
