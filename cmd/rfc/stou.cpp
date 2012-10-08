#include "cmd/rfc/stou.hpp"
#include "fs/file.hpp"
#include "cmd/factory.hpp"

namespace cmd { namespace rfc
{

void STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  if (!argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  fs::Path uniquePath;
  if (!fs::UniqueFile(client, client.WorkDir(),
                      filenameLength, uniquePath))
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to generate a unique filename.");
    return;
  }
  
  argStr = uniquePath.ToString();
  args.clear();
  args.emplace_back("STOR");
  args.emplace_back(argStr);
  
  ftp::ClientState reqdState;

  std::unique_ptr<cmd::Command>
    command(cmd::Factory::Create(client, argStr, args, reqdState));
  assert(command.get());
  command->Execute();
}

} /* rfc namespace */
} /* cmd namespace */
