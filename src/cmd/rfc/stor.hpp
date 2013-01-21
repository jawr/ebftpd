#ifndef __CMD_RFC_STOR_HPP
#define __CMD_RFC_STOR_HPP

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

class STORCommand : public Command
{
  bool CalcCRC(const fs::VirtualPath& path);
  void DupeMessage(const fs::VirtualPath& path);
  int Ratio(const fs::VirtualPath& path, 
      const boost::optional<const cfg::Section&>& section);
  
public:
  STORCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* rfc namespace */
} /* cmd namespace */

#endif
