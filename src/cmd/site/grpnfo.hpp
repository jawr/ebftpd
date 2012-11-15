#ifndef __CMD_SITE_GRPNFO_HPP
#define __CMD_SITE_GRPNFO_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{

class GRPNFOCommand : public Command
{
  static const std::string charsNotAllowed;
  static bool Valid(const std::string& nfoline);
  
public:
  GRPNFOCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
