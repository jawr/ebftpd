#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include "cmd/site/idle.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace site
{

void IDLECommand::Execute()
{
  namespace pt = boost::posix_time;

  static const char* syntax = "Syntax: SITE IDLE [<seconds>]";
  if (args.size() == 1)
  {
    std::ostringstream os;
    os << "Your current IDLE timeout is "
       << client.IdleTimeout().total_seconds() << " seconds.";
    control.Reply(ftp::CommandOkay, os.str());
  }                
  else if (args.size() != 2) control.Reply(ftp::SyntaxError, syntax);
  else
  {
    pt::seconds idleTimeout(0);
    try
    {
      idleTimeout = pt::seconds(boost::lexical_cast<long>(args[1]));
    }
    catch (const boost::bad_lexical_cast&)
    {
      control.Reply(ftp::SyntaxError, syntax);
      return;
    }
    
    const cfg::Config& config = cfg::Get();
    
    if (idleTimeout < config.IdleTimeout().Minimum() ||
        idleTimeout > config.IdleTimeout().Maximum())
    {
      std::ostringstream os;
      os << "IDLE timeout must be between " 
         << config.IdleTimeout().Minimum().total_seconds()
         << " and " << config.IdleTimeout().Maximum().total_seconds()
         << " seconds.";
      control.Reply(ftp::SyntaxError, os.str());
      return;
    }
    
    client.SetIdleTimeout(idleTimeout);
    std::ostringstream os;
    os << "IDLE timeout set to " << idleTimeout.total_seconds() 
       << " seconds.";
    control.Reply(ftp::CommandOkay, os.str());
  }
}

} /* site namespace */
} /* cmd namespace */
