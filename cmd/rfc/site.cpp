#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "cmd/rfc/site.hpp"
#include "cmd/site/factory.hpp"

namespace cmd { namespace rfc
{

void SITECommand::Execute()
{
  argStr = argStr.substr(args[0].length());
  boost::trim(argStr);
  args.erase(args.begin());
  boost::to_upper(args[0]);
  std::unique_ptr<cmd::Command>
    command(cmd::site::Factory::Create(client, argStr, args));
  if (!command.get()) control.Reply(ftp::CommandUnrecognised, "Command not understood");
  else command->Execute();
}

} /* rfc namespace */
} /* cmd namespace */
