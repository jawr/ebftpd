#ifndef __CMD_SITE_TRAFFIC_HPP
#define __CMD_SITE_TRAFFIC_HPP

#include <string>
#include "cmd/command.hpp"

namespace db { namespace stats
{
class ProtocolTotal;
}
}

namespace cmd { namespace site
{

class TRAFFICCommand : public Command
{
  std::string Format(const std::string& timeframe, long long sendBytes, 
            long long receiveBytes, const std::string& section = "");
            
public:
  TRAFFICCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
