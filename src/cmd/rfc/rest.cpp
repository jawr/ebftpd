#include <sstream>
#include "cmd/rfc/rest.hpp"

namespace cmd { namespace rfc
{

cmd::Result RESTCommand::Execute()
{
  off_t restart;
  
  try
  {
    restart = boost::lexical_cast<int>(args[1]);
    if (restart < 0) throw boost::bad_lexical_cast();
  }
  catch (const boost::bad_lexical_cast&)
  {
    control.Reply(ftp::InvalidRESTParameter, "Invalid parameter, restart offset set to 0.");
    data.SetRestartOffset(0);
    return cmd::Result::Okay;
  }
  
  data.SetRestartOffset(restart);
  
  std::ostringstream os;
  os << "Restart offset set to " << restart << ".";
  control.Reply(ftp::CommandOkay, os.str()); 
  return cmd::Result::Okay;
}

} /* rfc namespace */
} /* cmd namespace */
