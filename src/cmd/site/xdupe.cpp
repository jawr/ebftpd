#include <sstream>
#include "cmd/site/xdupe.hpp"
#include "ftp/xdupe.hpp"

namespace cmd { namespace site
{

cmd::Result XDUPECommand::Execute()
{
  if (args.size() == 1)
  {
    if (client.XDupeMode() == ftp::XDupeMode::Disabled)
      control.Reply(ftp::CommandOkay, "Extended dupe mode is disabled.");
    else
    {
      std::ostringstream os;
      os << "Extended dupe mode " << static_cast<int>(client.XDupeMode())
         << " is enabled.";
      control.Reply(ftp::CommandOkay, os.str());
    }
    return cmd::Result::Okay;
  }
  
  int mode;
  try
  {
    mode = boost::lexical_cast<int>(args[1]);
  }
  catch (const boost::bad_lexical_cast&)
  {
    return cmd::Result::SyntaxError;
  }
  
  if (mode < 0 || mode > 4)
  {
    return cmd::Result::SyntaxError;
  }
  
  client.SetXDupeMode(static_cast<ftp::XDupeMode>(mode));
  std::ostringstream os;
  os << "Activated extended dupe mode " << mode << ".";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
