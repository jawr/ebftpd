#include <boost/algorithm/string/case_conv.hpp>
#include "cmd/site/sreply.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void SREPLYCommand::Execute()
{
  if (args.size() == 1)
  {
    control.Reply(ftp::CommandOkay, "Single line reply mode is currently " +
          std::string(control.SingleLineReplies() ? "on" : "off") + ".");
  }                
  else
  {
    boost::to_lower(args[1]);
    if (args[1] == "on")
    {
      control.SetSingleLineReplies(true);
      control.Reply(ftp::SyntaxError, "Single line reply mode now set to on.");
    }
    else if (args[1] == "off")
    {
      control.SetSingleLineReplies(false);
      control.Reply(ftp::SyntaxError, "Single line reply mode now set to off.");
    }
    else
      throw cmd::SyntaxError();
  }
}

} /* site namespace */
} /* cmd namespace */
