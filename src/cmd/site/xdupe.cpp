#include <sstream>
#include "cmd/site/xdupe.hpp"
#include "ftp/xdupe.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void XDUPECommand::Execute()
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
    return;
  }
  
  int mode;
  try
  {
    mode = boost::lexical_cast<int>(args[1]);
  }
  catch (const boost::bad_lexical_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  if (mode < 0 || mode > 4)
  {
    throw cmd::SyntaxError();
  }
  
  client.SetXDupeMode(static_cast<ftp::XDupeMode>(mode));
  std::ostringstream os;
  os << "Activated extended dupe mode " << mode << ".";
  control.Reply(ftp::CommandOkay, os.str());
  return;
}

} /* site namespace */
} /* cmd namespace */
