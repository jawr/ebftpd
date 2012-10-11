#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/rfc/site.hpp"

namespace cmd { namespace rfc
{

void SITECommand::Execute()
{
  if (argStr.empty())
  {
    control.Reply(ftp::SyntaxError, "Wrong number of arguments.");
    return;
  }
  
  boost::to_upper(args[1]);
  if (args[1] == "EPSV")
  {
    static const char* syntax = "Syntax: SITE EPSV normal|full";
    if (args.size() == 2)
      control.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                   std::string(data.EPSVMode() == ftp::EPSVMode::Normal ?
                   "normal" : "full") + "'.");
                    
    else if (args.size() != 3) control.Reply(ftp::SyntaxError, syntax);
    else
    {
      boost::to_upper(args[2]);
      if (args[2] == "NORMAL")
      {
        data.SetEPSVMode(ftp::EPSVMode::Normal);
        control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'normal'.");
      }
      else if (args[2] == "FULL")
      {
        data.SetEPSVMode(ftp::EPSVMode::Full);
        control.Reply(ftp::SyntaxError, "Extended passive mode now set to 'full'.");
      }
      else
        control.Reply(ftp::SyntaxError, syntax);
    }
    return;
  }
  
  control.Reply(ftp::CommandUnrecognised, "SITE " + args[1] + 
               " command unrecognised."); 
}

} /* rfc namespace */
} /* cmd namespace */
