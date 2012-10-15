#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/epsv.hpp"

namespace cmd { namespace site
{

cmd::Result EPSVCommand::Execute()
{
  if (args.size() == 1)
  {
    control.Reply(ftp::CommandOkay, "Extended passive mode is currently '" +
                 std::string(data.EPSVMode() == ftp::EPSVMode::Normal ?
                 "normal" : "full") + "'.");
  }                
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
      return cmd::Result::SyntaxError;
  }
  return cmd::Result::Okay;
}

} /* site namespace */
} /* cmd namespace */
