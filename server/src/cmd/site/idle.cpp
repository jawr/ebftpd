#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "cmd/site/idle.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"

namespace cmd { namespace site
{

void IDLECommand::Execute()
{
  namespace pt = boost::posix_time;

  if (client.Profile().IdleTime() == 0)
  {
    control.Reply(ftp::CommandOkay, "This command doesn't apply to you, you have no idle limit.");
    return;
  }

  if (args.size() == 1)
  {
    std::ostringstream os;
    os << "Your current idle timeout is "
       << client.IdleTimeout().total_seconds() << " seconds.";
    control.Reply(ftp::CommandOkay, os.str());
  }                
  else
  {
    pt::seconds idleTimeout(0);
    try
    {
      idleTimeout = pt::seconds(boost::lexical_cast<long>(args[1]));
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
    
    const cfg::Config& config = cfg::Get();
    
    pt::seconds maximum(config.IdleTimeout().Maximum());
    if (client.Profile().IdleTime() > 0)
      maximum = pt::seconds(client.Profile().IdleTime());
    
    if (idleTimeout < config.IdleTimeout().Minimum() ||
        idleTimeout > maximum)
    {
      std::ostringstream os;
      os << "Idle timeout must be between " 
         << config.IdleTimeout().Minimum().total_seconds()
         << " and " << maximum.total_seconds() << " seconds.";
      control.Reply(ftp::SyntaxError, os.str());
      return;
    }
    
    client.SetIdleTimeout(idleTimeout);
    std::ostringstream os;
    os << "Idle timeout set to " << idleTimeout.total_seconds() 
       << " seconds.";
    control.Reply(ftp::CommandOkay, os.str());
  }
  
  return;
}

} /* site namespace */
} /* cmd namespace */
