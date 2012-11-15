#ifndef __CMD_SITE_CHGRP_HPP
#define __CMD_SITE_CHGRP_HPP

#include "cmd/command.hpp"

namespace cmd { namespace site
{


class CHGRPCommand : public Command
{
  enum class Method
  {
    Add,
    Delete,
    Default
  };

public:
  CHGRPCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif

