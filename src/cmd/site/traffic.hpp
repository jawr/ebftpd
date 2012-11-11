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
  std::string FormatProtocol(const std::string& timeframe, 
            const db::stats::ProtocolTotal& t);
            
public:
  TRAFFICCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  cmd::Result Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
