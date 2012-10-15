#include "cmd/rfc/stou.hpp"
#include "fs/file.hpp"
#include "cmd/rfc/factory.hpp"

namespace cmd { namespace rfc
{

cmd::Result STOUCommand::Execute()
{
  static size_t filenameLength = 10;

  fs::Path uniquePath;
  if (!fs::UniqueFile(client, client.WorkDir(),
                      filenameLength, uniquePath))
  {
    control.Reply(ftp::ActionNotOkay,
                 "Unable to generate a unique filename.");
    return cmd::Result::Okay;
  }
  
  argStr = uniquePath.ToString();
  args.clear();
  args.emplace_back("STOR");
  args.emplace_back(argStr);
  
  
  CommandPtr command(cmd::rfc::Factory::Lookup("STOU")->Create(client, argStr, args));
  assert(command.get());
  return command->Execute();
}

} /* rfc namespace */
} /* cmd namespace */
