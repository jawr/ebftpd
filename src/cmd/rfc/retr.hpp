#ifndef __CMD_RFC_RETR_HPP
#define __CMD_RFC_RETR_HPP

#include <boost/optional.hpp>
#include "cmd/command.hpp"

namespace fs
{
class VirtualPath;
}

namespace cfg
{
class Section;
}

namespace cmd { namespace rfc
{

class RETRCommand : public Command
{
  int Ratio(const fs::VirtualPath& path, 
        const boost::optional<const cfg::Section&>& section);
  
public:
  RETRCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
