#include "cmd/site/epsv.hpp"

namespace cmd { namespace site
{

void EPSVCommand::Execute()
{
  static const char* syntax = "Syntax: SITE EPSV normal|full";
  if (args.size() == 1)
  {
    control.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                 std::string(data.EPSVMode() == ftp::EPSVMode::Normal ?
                 "normal" : "full") + "'.");
  }                
  else if (args.size() != 2) control.Reply(ftp::SyntaxError, syntax);
  else
  {
    boost::to_upper(args[1]);
    if (args[1] == "NORMAL")
    {
      data.SetEPSVMode(ftp::EPSVMode::Normal);
      control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'normal'.");
    }
    else if (args[1] == "FULL")
    {
      data.SetEPSVMode(ftp::EPSVMode::Full);
      control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'full'.");
    }
    else
      control.Reply(ftp::SyntaxError, syntax);
  }
}

} /* site namespace */
} /* cmd namespace */
