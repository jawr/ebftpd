#include <sstream>
#include "cmd/site/xdupe.hpp"
#include "ftp/xdupe.hpp"

namespace cmd { namespace site
{

void XDUPECommand::Execute()
{
  static const char* syntax = "SITE XDUPE [<mode 0-4>]";
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
  
  if (args.size() != 2)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }
  
  int mode;
  try
  {
    mode = boost::lexical_cast<int>(args[1]);
  }
  catch (const boost::bad_lexical_cast&)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }
  
  if (mode < 0 || mode > 4)
  {
    control.Reply(ftp::SyntaxError, syntax);
    return;
  }
  
  client.SetXDupeMode(static_cast<ftp::XDupeMode>(mode));
  std::ostringstream os;
  os << "Activated extended dupe mode " << mode << ".";
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
