#ifndef __CMD_SITE_TAGLINE_HPP
#define __CMD_SITE_TAGLINE_HPP

#include <string>
#include "cmd/command.hpp"

namespace cmd { namespace site
{

class TAGLINECommand : public Command
{
  static const std::string charsNotAllowed;
  static bool Valid(const std::string& tagline);
public:
  TAGLINECommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
